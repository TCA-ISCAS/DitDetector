# -*- coding:utf-8 -*-
import os.path

import pandas as pd
import numpy as np
import torch
import warnings

from bimodal_preprocess import preprocess_join_fea
from bimodal_train import JointNet, MyPara
from sklearn.metrics import classification_report

warnings.filterwarnings("ignore")

device = torch.device('cuda' if torch.cuda.is_available() else 'cpu')
torch.cuda.set_device(0)
torch.cuda.empty_cache()


cur_dir = os.getcwd()
cur_data_dir = os.path.join(cur_dir, '..', '..', 'data', 'bimodal_res')
label_data_dir = cur_data_dir


def predict(val_data, val_name, net):
    net.eval()

    res_pred = None
    yhat_pred = None
    with torch.no_grad():
        for batch_idx, (x_data, y_data) in enumerate(val_data):
            X, labels = x_data.to(device), y_data.to(device)
            y_hat = net(X)

            y_hats, preds = torch.max(y_hat, 1)
            tmp_preds = preds.cpu().numpy()
            tmp_yhats = y_hats.cpu().numpy()

            if batch_idx == 0:
                res_pred = tmp_preds
                yhat_pred = tmp_yhats
            else:
                res_pred = np.concatenate((res_pred, tmp_preds), axis=0)
                yhat_pred = np.concatenate((yhat_pred, tmp_yhats), axis=0)

    df_md5 = pd.read_csv(os.path.join(label_data_dir, val_name))
    init_label = df_md5.label.to_list()

    res = classification_report(init_label, res_pred, target_names=['benign', 'malicious'], digits=4)
    print(res)

    return init_label, yhat_pred



def bimodal_detector(visual_representations, textual_representations):

    print('[Detect joint representations.]')

    # print(visual_representations.shape, textual_representations.shape)
    # print(type(visual_representations), type(textual_representations))

    my_para = MyPara()
    X = torch.cat([visual_representations, textual_representations], 1).cuda(device)

    # Load the model
    model_path = os.path.join(cur_data_dir, 'epoch_' + str(my_para.num_epochs) + '_joint_model.pt')
    net = JointNet(my_para.n_feature, my_para.n_hidden, my_para.n_output).cuda(device)
    state_dict = torch.load(model_path)
    net.load_state_dict(state_dict)
    # print(net)

    net.eval()
    with torch.no_grad():
            y_hat = net(X)

            y_hats, preds = torch.max(y_hat, 1)
            preds = preds.cpu().numpy()
            yhats = y_hats.cpu().numpy()

    print('[Done]')
    return preds





def main():
    my_para = MyPara()

    tar_flag = 'test'
    # tar_flag = 'xlm'
    # tar_flag = 'doc'
    if tar_flag == 'doc':
        my_para.val_name = 'doc.csv'
    elif tar_flag == 'xlm':
        my_para.val_name = 'xlm.csv'
    elif tar_flag == 'test':
        my_para.val_name = 'test_cl.csv'

    my_para.val_iter, my_para.n_feature = preprocess_join_fea(tar_flag, my_para.batch_size)

    # Load the model
    # model_path = cur_data_dir + 'epoch_' + str(my_para.num_epochs) + '_joint_model.pt'
    model_path = os.path.join(cur_data_dir, 'epoch_' + str(my_para.num_epochs) + '_joint_model.pt')
    net = JointNet(my_para.n_feature, my_para.n_hidden, my_para.n_output).cuda(device)
    state_dict = torch.load(model_path)
    net.load_state_dict(state_dict)
    print(net)

    # Predict
    predict(my_para.val_iter, my_para.val_name, net)


if __name__ == '__main__':
    main()
