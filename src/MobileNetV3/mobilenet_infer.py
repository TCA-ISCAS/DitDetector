import os
import torch
import numpy as np
from torch.autograd import Variable
import cv2
from collections import OrderedDict
from mobilenetv3 import mobilenetv3
from cvtransforms import cvtransforms
import json
import time


normalize = cvtransforms.Normalize(mean=[0.485, 0.456, 0.406],
                                   std=[0.229, 0.224, 0.225])

cur_dir = os.getcwd()
data_dir = os.path.join(cur_dir, '..', '..', 'data')


class SBP():
    def __init__(self, weight_path, num_classes=2, is_cuda=False, input_size=224):
        
        model = mobilenetv3(mode='small', n_class=num_classes)
        # print('Use GPU:',is_cuda)
        if is_cuda:
            state_dict = torch.load(weight_path)
            model = torch.nn.DataParallel(model)
            model.to(torch.device('cuda'))
            model.load_state_dict(state_dict,strict=True)
        else:
            state_dict = torch.load(weight_path, map_location='cpu')
            new_state_dict = OrderedDict()
            for key, value in state_dict.items():
                key = key.replace('module.', '')
                new_state_dict[key] = value
            model.load_state_dict(new_state_dict)
        model.eval()

        self.model = model
        self.is_cuda = is_cuda
        self.input_size = input_size
        
    def preprocess(self,image):
        transform = cvtransforms.Compose([
            cvtransforms.PreRatioResizeAndPad(self.input_size),
            cvtransforms.ToTensor(),
            normalize,])
        image_tensor = transform(image).float()
        if self.is_cuda:
            img = Variable(image_tensor.cuda())
        else:
            img = Variable(image_tensor)
        img.unsqueeze_(0)
        return img


    def softmax_score(self,pred):
        tmp = [np.e**i for i in pred]
        score = [i/sum(tmp) for i in tmp]
        return score

    def predict(self,img):  
        img = self.preprocess(img)
        with torch.no_grad():
            output = self.model(img)
        outputs = output.cpu().data
        temp = np.array(outputs[0])
        index = outputs.argmax()
        score = self.softmax_score(temp)
        return int(index),score

    def _get_features_from(self, x, feature_names):
        features = {}

        def save_feature(name):
            def hook(m, i, o):
                features[name] = o.data

            return hook

        for name, module in self.model.named_modules():
            # _name = name.split('.')[-1]
            _name = name
            if _name in feature_names:
                module.register_forward_hook(save_feature(_name))

        self.model(x)

        return features

    def get_feature(self,img):
        if self.is_cuda:
            out_layer = 'module.features.13'
        else:
            out_layer = 'features.13'
        img = self.preprocess(img)
        features = self._get_features_from(img, [out_layer])
        feature = features[out_layer].cpu()
        feat = np.reshape(feature, (1, 576))
        feat = feat/np.linalg.norm(feat)
        return feat


def get_visual_fea_representation(image_paths):

    print('[Generating the visual feature representations...]')

    weights_dir = os.path.join(data_dir, 'mobilenetv3', 'pretrained_weights')
    weight_path = os.path.join(weights_dir, '24_0.9896.pth')
    num_classes = 2
    input_size = 224
    is_cuda = torch.cuda.is_available()

    sbp = SBP(weight_path=weight_path, num_classes=num_classes, is_cuda=is_cuda, input_size=input_size)

    '''
        test images named with prefix '0_', '1_', '2_', 
        where '0_' means benign, '1_' and '2_' means malicious, 
        but model fails to predict '2_x.tiff'
    '''

    ft_list = []
    # test_image_path_1 = os.path.join(data_dir, 'mobilenetv3', 'test_images', '0_1.tiff')
    # test_image_path_2 = os.path.join(data_dir, 'mobilenetv3', 'test_images', '2_2.tiff')
    # image_paths = [test_image_path_1, test_image_path_2]
    for image_path in image_paths:
        im_arr = cv2.imread(image_path)
        '''
            predict: prediction, 1 for malicious samples, 0 for benign samples
            get_feature: get the feature (1,576) for each sample
        '''
        ft = sbp.get_feature(im_arr)
        # label, scores = sbp.predict(im_arr)
        # print(label, scores)
        ft_list.append(ft)

    features = torch.cat(ft_list, 0)
    print('[Done]')

    return features

#
# if __name__ == '__main__':
#
#     weights_dir = os.path.join(data_dir, 'mobilenetv3', 'pretrained_weights')
#     weight_path = os.path.join(weights_dir, '24_0.9896.pth')
#     input_size = 224
#     num_classes = 2
#     is_cuda = torch.cuda.is_available()
#
#     sbp = SBP(weight_path=weight_path, num_classes=num_classes, is_cuda=is_cuda, input_size=input_size)
#
#     '''
#         test images named with prefix '0_', '1_', '2_',
#         where '0_' means benign, '1_' and '2_' means malicious,
#         but model fails to predict '2_x.tiff'
#     '''
#     # image_path = data_dir + 'test_images/0_2.tiff'
#     # image_path = data_dir + 'test_images/1_2.tiff'
#     image_path = data_dir + '/test_images/2_2.tiff'
#     im_arr = cv2.imread(image_path)
#
#     '''
#         predict: prediction, 1 for malicious samples, 0 for benign samples
#         get_feature: get the feature (1,576) for each sample
#     '''
#     ft = sbp.get_feature(im_arr)
#     label, scores = sbp.predict(im_arr)
#     print(ft)
#     print(label, scores)
#
