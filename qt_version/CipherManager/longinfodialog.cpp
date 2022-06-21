﻿#include "longinfodialog.h"
#include "ui_longinfodialog.h"

LongInfoDialog::LongInfoDialog(const QString& title, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::LongInfoDialog)
{
    ui->setupUi(this);
    connect(this->ui->OKButton, this->ui->OKButton->clicked, this, reject);

    ui->titleLabel->setText(title);
    ui->textEdit->setDocumentTitle(title);
    this->setWindowTitle(title);

    if (title.compare("关于我") == 0) {
        ui->textEdit->setHtml("<div><h1>欢迎使用这个简单的密码管理软件</h1><p>市面上常用的密码管理软件有各种各样的问题，比如要钱、需要注册、有一大堆乱七八糟花里胡哨但是没有什么用的功能。最重要的是，这些软件往往有网络通信模块，谁也不能够保证软件背后的开发者不会在里面植入后门。我在Chrome里面放密码是不得已而为之，但是显然应该尽量少地暴露自己的密码。</p><p>本软件只有本地模块，单机使用。加密使用SHA256与AES算法，能够基本保障存储的账户密码的安全。</p><hr><h1>更新信息</h1><p>本软件使用Qt框架编写。由于我对Qt的使用还不太熟练，所以界面不太好看。在后续的更新中（如果有的话），我会对界面进行美化。</p><p>相比于早期的python版本（源码见Github），该版本添加了条目分组和备注的功能，如果你愿意，也可以将本软件作为一个加密记事本使用。</p><hr><h1>联系方式</h1><p>本软件源码公开在Github上</p><p><a href=\"https://github.com/Hermethus/CipherManager\">https://github.com/Hermethus/CipherManager</a></p><p>如果发现任何bug，欢迎邮件交流。</p><p>邮箱：lin_dian@foxmail.com</p><p>——Hermethus</p><p>2022/6/20</p></div>");
    } else if (title.compare("帮助") == 0) {
        ui->textEdit->setHtml("<div><h1>这个软件是做什么的</h1><p>这是一个密码本管理软件，可以加密保存你输入的信息，并在你需要的时候提供检索。软件会将加密信息保存在.cipherbook密码本文件中，所以请妥善保存好该文件，可以的话请进行备份。</p><p>同时，请牢记解锁密码本的密码，如果丢失了密码，那么就无法打开加密的密码本。</p><h1>开始使用</h1><p>从菜单栏的“文件-&gt;新建”选项开始密码本的使用。</p><p>本软件提供增加、修改、查找、删除功能。</p><p>每一条账户信息都包含以下属性：</p><ul><li>标识：必填。用于标注该条信息的归属、网站或者应用，如“QQ号”、“银行卡”、“支付宝”等。</li><li>别名：用于辅助搜索。可以通过搜索别名中的关键词来找到这一条。</li><li>分组：同一分组下的条目会集中显示。搜索对分组中的关键词同样有效。不填写分组则会统一归入“未分组”分组下。</li><li>账户名：网站的账号或者银行卡号等。</li><li>密码：加密显示。可以通过复制按钮来获取。</li><li>备注：其他备注信息。可以用来做备忘录之类功能。</li></ul><p>考虑到标识本身也可能属于敏感信息，本软件不提供默认的账户信息列表。使用方法类似搜索引擎，一切内容通过关键词搜索获取，关键词大小写不敏感。如果忘记了关键词，可以在搜索框中输入星号“*”来显示全部条目。</p><p>本软件提供明文导出功能，使用JSON格式存储。</p></div>");
    }
}

LongInfoDialog::~LongInfoDialog()
{
    delete ui;
}
