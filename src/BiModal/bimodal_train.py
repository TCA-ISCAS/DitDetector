# -*- coding:utf-8 -*-
import time
import torch
import torch.nn as nn
from bimodal_preprocess import preprocess_join_fea

torch.cuda.set_device(0)
torch.cuda.empty_cache()
device = torch.device('cuda' if torch.cuda.is_available() else 'cpu')

cur_data_dir = '../../data/bimodal_res/'


class JointNet(nn.Module):
    def __init__(self, n_features, n_hidden, n_output):
        super(JointNet, self).__init__()  # 继承父类的init方法
        self.fc1 = nn.Linear(n_features, n_hidden)  # 全连接层的搭建
        self.fc2 = nn.Linear(n_hidden, n_hidden)  # 全连接层的搭建
        self.dropout = nn.Dropout(0.5)
        self.relu = nn.ReLU()
        self.fc3 = nn.Linear(n_hidden, n_output)

    def forward(self, x):
        x = self.fc1(x)
        x = self.relu(x)
        x = self.fc2(x)
        x = self.relu(x)
        x = self.fc3(x)
        return x


def train(train_iter, test_iter, net, loss, optimizer, num_epochs, fp):
    print('[Training...]')
    for epoch in range(num_epochs):
        train_l_sum, train_acc_sum, n, start = 0.0, 0.0, 0, time.time()
        batch_count = 0
        for batch_idx, (x_data, y_data) in enumerate(train_iter):
            X, y = x_data.to(device), y_data.to(device)
            y_hat = net(X)
            l = loss(y_hat, y)
            optimizer.zero_grad()
            l.backward()
            optimizer.step()
            train_l_sum += l.item()
            train_acc_sum += (y_hat.argmax(dim=1) == y).sum().item()
            n += y.shape[0]
            batch_count += 1
        test_acc, test_lost = evaluate_accuracy(test_iter, net, loss)

        cur_lost = train_l_sum / batch_count
        cur_train_acc = train_acc_sum / n
        print(
            'epoch %d, loss %.4f, train acc %.3f, val acc %.3f, val loss %.3f, time %.1f sec'
            % (epoch + 1, cur_lost, cur_train_acc,
               test_acc, test_lost, time.time() - start))
        fp.write('epoch %d, loss %.6f, train acc %.6f, val acc %.6f, val loss %.6f, time %.6f sec'
                 % (epoch + 1, cur_lost, cur_train_acc,
                    test_acc, test_lost, time.time() - start))
        fp.write('\n')


def evaluate_accuracy(train_iter, net, loss):
    acc_sum, n = 0.0, 0
    l_sum = 0.0
    batch_count = 0
    with torch.no_grad():
        for batch_idx, (x_data, y_data) in enumerate(train_iter):
            X, y = x_data.to(device), y_data.to(device)
            if isinstance(net, torch.nn.Module):
                net.eval()
                y_hat = net(X)
                acc_sum += (y_hat.argmax(dim=1) == y).float().sum().item()
                l = loss(y_hat, y)
                l_sum += l.item()
                net.train()
            else:
                if ('is_training' in net.__code__.co_varnames):
                    acc_sum += (net(X, is_training=False).argmax(dim=1) == y).float().sum().item()
                else:
                    acc_sum += (net(X).argmax(dim=1) == y).float().sum().item()
            batch_count += 1
            n += y.shape[0]
            size = len(train_iter.dataset)
    return acc_sum / n, l_sum / size


class MyPara():
    def __init__(self):
        self.batch_size = 64
        self.kernel_sizes = [3, 4, 5]
        self.num_channels = [100, 100, 100]
        self.train_name = 'train_4_1_cl.csv'
        self.val_name = 'val_cl.csv'
        self.train_data = None
        self.val_data = None
        self.train_iter = None
        self.val_iter = None
        self.vocab_size = None
        self.embedding_dim = None
        self.lr = 0.001
        self.num_epochs = 100
        self.device = device
        self.n_feature = 876
        self.n_hidden = 100
        self.n_output = 2


def main():

    my_para = MyPara()
    my_para.train_iter, my_para.n_feature = preprocess_join_fea('train_4_2', my_para.batch_size)
    my_para.val_iter, _ = preprocess_join_fea('val', my_para.batch_size)

    net = JointNet(my_para.n_feature, my_para.n_hidden, my_para.n_output)
    print(net)
    net.to(device)

    optimizer = torch.optim.Adam(net.parameters(), lr=my_para.lr)
    loss = nn.CrossEntropyLoss()
    fp = open(cur_data_dir + 'training_log.txt', 'w')
    train(my_para.train_iter, my_para.val_iter, net, loss, optimizer, my_para.num_epochs, fp)
    fp.close()

    model_path = cur_data_dir + 'epoch_' + str(my_para.num_epochs) + '_joint_model.pt'
    torch.save(net.state_dict(), model_path)


if __name__ == "__main__":
    main()
