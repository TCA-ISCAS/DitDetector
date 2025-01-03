# -*- coding:utf-8 -*-
import os

import h5py
import pandas as pd
import numpy as np
import torch

from text_preprocess_doc import preprocess_docs
from textcnn_train import TextCNN, MyPara
from sklearn.metrics import classification_report
from torchtext import data

import warnings

warnings.filterwarnings("ignore")

cur_dir = os.getcwd()
cur_data_dir = os.path.join(cur_dir, '..', '..', 'data', 'text_cnn_res') + '/'

device = torch.device('cuda' if torch.cuda.is_available() else 'cpu')
torch.cuda.set_device(0)
torch.cuda.empty_cache()


def predict(val_data, val_name, net):
    net.eval()

    res_pred = None
    init_label = None
    with torch.no_grad():
        for batch_idx, batch in enumerate(val_data):
            X, labels = batch.text, batch.label
            X = X.permute(1, 0)
            labels.data.sub_(1)
            y_hat = net(X)

            _, preds = torch.max(y_hat, 1)

            tmp_preds = preds.cpu().numpy()
            tmp_labels = labels.cpu().numpy()

            if batch_idx == 0:
                res_pred = tmp_preds
                init_label = tmp_labels
            else:
                res_pred = np.concatenate((res_pred, tmp_preds), axis=0)
                init_label = np.concatenate((init_label, tmp_labels), axis=0)

    res_pred = [e ^ 1 for e in res_pred]
    init_label = [e ^ 1 for e in init_label]

    res = classification_report(init_label, res_pred, target_names=['benign', 'malicious'], digits=4)
    print(res)


def extract_fea(val_iter, net):
    features = []
    net.eval()

    def pre_hook(module, input):
        print('\n[hooking...]')
        features.append(input[0])

    with torch.no_grad():
        for batch_idx, batch in enumerate(val_iter):
            X, labels = batch.text, batch.label
            print(X)
            print(X.shape)
            X = X.permute(1, 0)
            labels.data.sub_(1)

            handle = net.decoder.register_forward_pre_hook(pre_hook)
            y_hat = net(X)
            handle.remove()

    return features


def transfer_tensor_list(fea_list, file_path):
    fea_res = None
    for idx, fea_tensor in enumerate(fea_list):
        tmp_fea = fea_tensor.cpu().numpy()  # detach().numpy()

        if idx == 0:
            fea_res = tmp_fea
        else:
            fea_res = np.concatenate((fea_res, tmp_fea), axis=0)

    df_data = pd.read_csv(file_path)
    md5_list = df_data.md5.to_numpy()
    label_list = df_data.label.to_numpy()

    tar_data_dir = os.path.join(cur_data_dir, '..', 'fea_presentation', 'textcnn_features')
    if not os.path.exists(tar_data_dir):
        os.mkdir(tar_data_dir)
    tar_file_path = tar_data_dir + file_path.split('/')[-1].split('.')[0] + '.hdf5'

    # h5f = h5py.File(tar_file_path, 'w')
    # h5f.create_dataset('ocr_fea_rep', data=fea_res)
    # h5f.create_dataset('md5', data=md5_list)
    # h5f.create_dataset('label', data=label_list)
    # h5f.close()


def generate_fea_rep(data_iter, data_name, net):
    tmp_fea = extract_fea(data_iter, net)
    tmp_file_path = os.path.join(cur_data_dir, data_name)
    transfer_tensor_list(tmp_fea, tmp_file_path)


def val_generator(my_para):
    val_iter = data.Iterator(dataset=my_para.val_data, batch_size=my_para.batch_size, shuffle=False,
                             sort=False, repeat=False, device=my_para.device)
    return val_iter


def main():
    my_para = MyPara()

    op_flag = 'generate_fea'  # extract the encoded feature representation
    # op_flag = 'predict'  # predict the maliciousness

    my_para.val_name = 'test_cl.csv'
    # my_para.val_name = 'doc.csv'
    # my_para.val_name = 'xlm.csv'

    my_para.train_data, my_para.val_data, my_para.vocab_size, my_para.embedding_dim = preprocess_docs(
        my_para.train_name,
        my_para.val_name,
    )
    my_para.val_iter = val_generator(my_para)

    net = TextCNN(my_para.vocab_size, my_para.embedding_dim, my_para.kernel_sizes, my_para.num_channels).cuda(device)
    # load the model and predict test dataset
    model_path = os.path.join(cur_data_dir, 'epoch_' + str(my_para.num_epochs) + '_textcnn_model.pt')
    state_dict = torch.load(model_path)
    net.load_state_dict(state_dict)
    print(net)

    if op_flag == 'predict':
        predict(my_para.val_iter, my_para.val_name, net)
    elif op_flag == 'generate_fea':
        generate_fea_rep(my_para.val_iter, my_para.val_name, net)


class text_encoder():
    def __init__(self):
        self.batch_size = 64
        self.kernel_sizes = [3, 4, 5]
        self.num_channels = [100, 100, 100]
        # self.train_name = 'train_4_1_cl.csv'
        # self.val_name = 'val_cl.csv'
        # self.train_data = None
        # self.val_data = None
        # self.train_iter = None
        # self.val_iter = None
        self.vocab_size = 44369
        self.embedding_dim = 200
        # self.lr = 0.001
        self.num_epochs = 20
        self.device = device
        self.net = None
        self.doc_embeddings = None
        self.fea_representation = None

    def load_text_encoder(self):
        net = TextCNN(self.vocab_size, self.embedding_dim, self.kernel_sizes, self.num_channels).cuda(self.device)
        model_path = os.path.join(cur_data_dir, 'epoch_' + str(self.num_epochs) + '_textcnn_model.pt')
        state_dict = torch.load(model_path)
        net.load_state_dict(state_dict)
        # print(net)
        return net

    def get_fea_representation(self, doc_embeddings):

        self.doc_embeddings = self.test_generator(doc_embeddings)
        features = []

        net = self.load_text_encoder()
        net.eval()

        def pre_hook(module, input):
            # print('\n[hooking...]')
            features.append(input[0])

        with torch.no_grad():
            for batch_idx, batch in enumerate(self.doc_embeddings):
                X = batch.text
                X = X.permute(1, 0)

                handle = net.decoder.register_forward_pre_hook(pre_hook)
                y_hat = net(X)
                handle.remove()

        self.fea_representation = self.transfer_tensor_list(features)
        # print(type(self.fea_representation))
        return self.fea_representation

    def test_generator(self, doc_embeddings):
        test_iter = data.Iterator(dataset=doc_embeddings, batch_size=self.batch_size, shuffle=False,
                                 sort=False, repeat=False, device=self.device)
        return test_iter

    def transfer_tensor_list(self, fea_list):
        fea_res = None
        for idx, fea_tensor in enumerate(fea_list):
            # tmp_fea = fea_tensor.cpu().numpy()
            tmp_fea = fea_tensor.cpu()

            if idx == 0:
                fea_res = tmp_fea
            else:
                fea_res = np.concatenate((fea_res, tmp_fea), axis=0)
        fea_res = torch.tensor(fea_res)
        return fea_res


if __name__ == '__main__':
    main()
