# -*- coding:utf-8 -*-
import os, sys

import h5py
import numpy as np
import pandas as pd
import torch

cur_dir = os.getcwd()
src_path = os.path.abspath(os.path.join(cur_dir, '..'))
sys.path.append(src_path)

from DitDetector.basic_func import traverse_files
from torch.utils.data import Dataset
from torch.utils.data import DataLoader

cur_data_dir = '../../data/'
g_random_state = 33

device = torch.device('cuda' if torch.cuda.is_available() else 'cpu')


def load_data(data_flag):

    retrained_fea_dir = cur_data_dir + 'fea_presentation/'
    cv_dir = retrained_fea_dir + 'cv_features'
    nlp_dir = retrained_fea_dir + 'textcnn_features'

    cv_file_list = traverse_files(cv_dir)
    nlp_file_list = traverse_files(nlp_dir)

    cv_path = [e for e in cv_file_list if data_flag in e][0]
    nlp_path = [e for e in nlp_file_list if data_flag in e][0]

    cv_h5f = h5py.File(cv_path, 'r')
    nlp_h5f = h5py.File(nlp_path, 'r')

    cv_md5 = [e.decode('utf8').split('.')[0] for e in cv_h5f['img_names'][:]]
    cv_feats = cv_h5f['feats']

    nlp_feats = nlp_h5f['ocr_fea_rep']
    nlp_label = nlp_h5f['label']

    join_feats = np.concatenate((cv_feats, nlp_feats), axis=1).tolist()

    '''
        cv and nlp have shared and sorted md5
        nlp_label is the initial labels stem from MalDoc
        cv_label is cv gt label by Wan
    '''
    df_data = pd.DataFrame({'md5': cv_md5, 'fea': join_feats, 'label': nlp_label})

    return df_data


def my_collate(batch_data):

    x_data = []
    y_data = []
    for sample in batch_data:
        x_data.append(torch.FloatTensor(sample[0]))
        y_data.append(sample[1])

    return torch.stack(x_data), torch.LongTensor(y_data)


class MyDataset(Dataset):
    def __init__(self, df):
        self.x_data = df['fea'].to_numpy()
        self.y_data = df['label'].to_numpy()
        self.length = len(self.y_data)
        self.dim = len(df['fea'].values[0])

    def __getitem__(self, idx):
        return self.x_data[idx], self.y_data[idx]

    def __len__(self):
        return self.length


def preprocess_join_fea(tar_data_flag, batch_size):

    df_data = load_data(tar_data_flag)
    my_dataset = MyDataset(df_data)

    # shuffle_flag = True
    shuffle_flag = False
    if 'test' in tar_data_flag:
        shuffle_flag = False
    data_loader = DataLoader(dataset=my_dataset,
                             batch_size=batch_size,
                             shuffle=shuffle_flag,
                             collate_fn=my_collate,
                             num_workers=0)

    return data_loader, my_dataset.dim


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


