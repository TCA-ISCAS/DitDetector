# -*- coding:utf-8 -*-
import os
import time

import torch
import torch.nn as nn
import torch.nn.functional as F

from text_preprocess_doc import preprocess_docs
from torchtext import data

cur_dir = os.getcwd()
cur_data_dir = os.path.join(cur_dir, '..', '..', 'data', 'text_cnn_res')

device = torch.device('cuda' if torch.cuda.is_available() else 'cpu')
torch.cuda.set_device(0)
torch.cuda.empty_cache()


class GlobalMaxPool1d(nn.Module):
    def __init__(self):
        super(GlobalMaxPool1d, self).__init__()

    def forward(self, x):
        return F.max_pool1d(x, kernel_size=x.shape[2])  # shape: (batch_size, channel, 1)


class TextCNN(nn.Module):
    def __init__(self, vocab_size, embedding_dim, kernel_size, num_channels):
        super(TextCNN, self).__init__()
        self.word_embeddings = nn.Embedding(vocab_size, embedding_dim)
        self.dropout = nn.Dropout(0.5)
        self.decoder = nn.Linear(sum(num_channels), 2)  # fc 全连接层
        self.pool = GlobalMaxPool1d()
        self.convs = nn.ModuleList()
        for c, k in zip(num_channels, kernel_size):
            self.convs.append(nn.Conv1d(in_channels=embedding_dim,
                                        out_channels=c,
                                        kernel_size=k))

    def forward(self, sentence):
        embeds = self.word_embeddings(sentence)
        embeds = embeds.permute(0, 2, 1)
        encoding = torch.cat([self.pool(F.relu(conv(embeds))).squeeze(-1) for conv in self.convs], dim=1)
        outputs = self.decoder(self.dropout(encoding))
        return outputs


def train(train_iter, test_iter, net, loss, optimizer, num_epochs, fp):
    for epoch in range(num_epochs):
        train_l_sum, train_acc_sum, n, start = 0.0, 0.0, 0, time.time()
        batch_count = 0
        for batch_idx, batch in enumerate(train_iter):
            X, y = batch.text, batch.label
            X = X.permute(1, 0)
            y.data.sub_(1)
            y_hat = net(X)
            l = loss(y_hat, y)
            optimizer.zero_grad()
            l.backward()
            optimizer.step()
            train_l_sum += l.item()
            train_acc_sum += (y_hat.argmax(dim=1) == y).sum().item()
            n += y.shape[0]
            batch_count += 1
        test_acc, test_loss = evaluate_accuracy(test_iter, net, loss)

        cur_loss = train_l_sum / batch_count
        cur_train_acc = train_acc_sum / n
        print(
            'epoch %d, loss %.4f, train acc %.3f, test acc %.3f, test loss %.3f, time %.1f sec'
            % (epoch + 1, cur_loss, cur_train_acc,
               test_acc, test_loss, time.time() - start))
        fp.write('epoch %d, loss %.6f, train acc %.6f, test acc %.6f, test loss %.6f, time %.6f sec'
                 % (epoch + 1, cur_loss, cur_train_acc,
                    test_acc, test_loss, time.time() - start))
        fp.write('\n')


def evaluate_accuracy(train_iter, net, loss):
    acc_sum, n = 0.0, 0
    l_sum = 0.0
    batch_count = 0
    with torch.no_grad():
        for batch_idx, batch in enumerate(train_iter):
            X, y = batch.text, batch.label
            X = X.permute(1, 0)
            y.data.sub_(1)
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
        self.vocab_size = 44369
        self.embedding_dim = 200
        self.lr = 0.001
        self.num_epochs = 20
        self.device = device


def main():
    my_para = MyPara()

    my_para.train_data, my_para.val_data, my_para.vocab_size, my_para.embedding_dim = preprocess_docs(
        my_para.train_name,
        my_para.val_name
    )

    my_para.train_iter, my_para.val_iter = data.Iterator.splits((my_para.train_data, my_para.val_data),
                                                                batch_sizes=(my_para.batch_size, my_para.batch_size),
                                                                device=device,
                                                                sort_key=lambda x: len(x.text),
                                                                sort_within_batch=False,
                                                                repeat=False)

    train_log_file_name = str(my_para.num_epochs) + '_training_log.txt'
    log_path = os.path.join(cur_data_dir, train_log_file_name)
    fp = open(log_path, 'w')

    model_name = 'epoch_' + str(my_para.num_epochs) + '_textcnn_model.pt'
    model_path = os.path.join(cur_data_dir, model_name)
    net = None
    if not os.path.exists(model_path):
        net = TextCNN(my_para.vocab_size, my_para.embedding_dim, my_para.kernel_sizes, my_para.num_channels).cuda(
            device)
        print(net)
        net.cuda()

        optimizer = torch.optim.Adam(net.parameters(), lr=my_para.lr)
        loss = nn.CrossEntropyLoss()
        train(my_para.train_iter, my_para.val_iter, net, loss, optimizer, my_para.num_epochs, fp)
        torch.save(net.state_dict(), model_path)
    fp.close()


if __name__ == '__main__':
    # main()
    print('hello')
