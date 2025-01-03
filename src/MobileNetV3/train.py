# !/usr/bin/env python
# -*- encoding: utf-8 -*-
import os
import time
from pathlib import Path

import cv2
import torch
import torch.nn as nn
import torch.optim as optim
from torch.autograd import Variable
from torch.utils.data.dataset import Dataset
from tqdm import tqdm

from cvtransforms import cvtransforms
from mobilenetv3 import mobilenetv3
from collections import OrderedDict

os.environ["CUDA_VISIBLE_DEVICES"] = "2,3"

label_dict = {'0': 0,
              '1': 1}


class SimpleDataset(Dataset):
    def __init__(self, image_dir, img_transforms):
        super(SimpleDataset, self).__init__()
        self.transforms = img_transforms
        self.images = list(Path(image_dir).rglob('*.*'))
        self.labels = None

    def __getitem__(self, item):
        image = cv2.imread(str(self.images[item]))
        label = label_dict[self.images[item].parent.name]
        image = self.transforms(image)

        return image, label

    def __len__(self):
        return len(self.images)


def train(training_data):
    train_loss = 0
    train_acc = 0

    for x, y in tqdm(training_data, desc='train',
                     total=len(training_data)):
        model.train()

        x = Variable(x.cuda())
        y = Variable(y.cuda())

        optimizer.zero_grad()

        output = model(x)
        loss = loss_func(output, y)
        train_loss += loss.item()

        loss.backward()
        optimizer.step()

        train_acc += (output.argmax(1) == y).sum().item()
    return train_loss / train_num, train_acc / train_num


def val(val_data):
    loss = 0
    acc = 0
    model.eval()
    for x, y in tqdm(val_data, desc='val',
                     total=len(val_data)):
        x = Variable(x.cuda())
        y = Variable(y.cuda())

        with torch.no_grad():
            output = model(x)

            loss = loss_func(output, y)
            loss += loss.item()

            acc += (output.argmax(1) == y).sum().item()
    return loss / val_num, acc / val_num


train_num = 3484   # num of img to train
val_num = 1057   # num of img to validate

input_size = 224
N_EPOCHS = 100
batch_size = 512
n_worker = 8
num_classes = 2


data_dir = '../../data/mobilenetv3/'
train_dir = data_dir + 'train4_1/'
val_dir = data_dir + 'val/'
model_save_path = data_dir + 'datapretrained_weight/'
os.makedirs(model_save_path, exist_ok=True)

normalize = cvtransforms.Normalize(mean=[0.485, 0.456, 0.406],
                                   std=[0.229, 0.224, 0.225])

# Load data
train_transform = cvtransforms.Compose([
        
        cvtransforms.RandomChooseTwo(
                [
            cvtransforms.RandomErasing(p=0.5),
            cvtransforms.ColorJitter(brightness=0.5,contrast=0.5,saturation=0.5,hue=0.1),
            cvtransforms.RandomGrayscale(p=0.2),
            cvtransforms.RandomGaussianNoise(p=0.4),
            cvtransforms.RandomPoissonNoise(p=0.4),
            cvtransforms.RandomSPNoise(p=0.4),
            cvtransforms.RandomAffine(degrees=10,translate=(0.25,0.25),scale=(0.9,1.1),shear=10)
                ]),
        cvtransforms.PreRatioResizeAndPad(input_size),
        cvtransforms.ToTensor(),
        normalize,
])
train_dataset = SimpleDataset(train_dir, train_transform)
train_loader = torch.utils.data.DataLoader(train_dataset,
                                           batch_size=batch_size,
                                           shuffle=True,
                                           num_workers=n_worker,
                                           pin_memory=True)

val_transform = cvtransforms.Compose([
        cvtransforms.PreRatioResizeAndPad(input_size),
        cvtransforms.ToTensor(),
        normalize,
])

val_dataset = SimpleDataset(val_dir, val_transform)
val_loader = torch.utils.data.DataLoader(val_dataset,
                                         batch_size=batch_size,
                                         shuffle=False,
                                         num_workers=n_worker,
                                         pin_memory=True)

# Load the model
model = mobilenetv3(mode='small')
model.classifier = nn.Sequential(
    nn.Dropout(p=0.5),    # refer to paper section 6
    nn.Linear(1280, num_classes),
)

# region: load imagenet pretrained modal
weight_path = 'pretrained_weights/v6_36_0.9886.pth'

is_cuda = torch.cuda.is_available()
print('Use GPU:', is_cuda)
if is_cuda:
    state_dict = torch.load(weight_path)
    model = torch.nn.DataParallel(model)
    model.to(torch.device('cuda'))
    model.load_state_dict(state_dict, strict=True)
else:
    state_dict = torch.load(weight_path, map_location='cpu')
    new_state_dict = OrderedDict()
    for key, value in state_dict.items():
        key = key.replace('module.', '')
        new_state_dict[key] = value
    model.load_state_dict(new_state_dict)

loss_func = nn.CrossEntropyLoss()
optimizer = optim.Adam(model.parameters())

best_test_acc = 0
for epoch in range(N_EPOCHS):
    start_time = time.time()
    train_loss, train_acc = train(train_loader)
    valid_loss, valid_acc = val(val_loader)
    best_test_acc = max(valid_acc, best_test_acc)

    secs = int(time.time() - start_time)
    mins = secs / 60
    secs = secs % 60
    print('Epoch: %d' % (epoch + 1),
          " | time in %d minutes, %d seconds" % (mins, secs))
    print(
        f'\tLoss: {train_loss:.4f}(train)\t|\tAcc: {train_acc * 100:.1f}%(train)')
    print(
        f'\tLoss: {valid_loss:.4f}(test)\t|\tAcc: {valid_acc * 100:.1f}%(test)')
    torch.save(model.state_dict(),
               f'{model_save_path}/{epoch}_{valid_acc:.4f}.pth')

print(f'Best Testing Acc: {best_test_acc * 100:.1f}% ')
