/**
 * Copyright (C) 2015 Shindo
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "mainwindow.h"

#include <QApplication>
#include <QStyle>
#include <QMessageBox>
#include <QDesktopWidget>
#include <QStringList>
#include <QProcess>
#include <QProcessEnvironment>

#include <unistd.h>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    // check user identity
    if (!getuid()) // as root
    {
        MainWindow w;

        // center window & set version
        w.setGeometry(QStyle::alignedRect(Qt::LeftToRight, Qt::AlignCenter, w.size(), a.desktop()->availableGeometry()));
        w.setWindowTitle(QString("EasyDrcomGUI For Linux, v" CURRENT_VERSION));

        w.show();
        return a.exec();
    }
    else
    {
        /*auto ret = QMessageBox::information(NULL, QString("EasyDrcomGUI"), QString("EasyDrcomGUI需要以root权限运行。\n是否提升至root权限？"),
                                            QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes);
        if (ret == QMessageBox::Yes)
        {
            QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
            QString display = env.value("DISPLAY");
            QString xauthority = env.value("XAUTHORITY");

            if (!display.isEmpty() && !xauthority.isEmpty())
            {
                auto ret = QMessageBox::information(NULL, QString("EasyDrcomGUI"), QString("是否使用pkexec方式提权？\n"
                                                                                           "当pkexec方式提权失败时，请尝试sudo方式。\n\n"
                                                                                           "点击\"是(Yes)\"以pkexec方式提权；\n"
                                                                                           "点击\"否(No)\"以sudo方式提权；\n"
                                                                                           "点击\"取消(Cancel)\"以退出。\n\n"
                                                                                           "若尝试sudo方式亦失败，请您在终端中执行如下命令：\n"
                                                                                           "      sudo /usr/bin/EasyDrcomGUI"),
                                                    QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel, QMessageBox::Yes);

                if (ret == QMessageBox::Yes)
                {
                    // okay, we can use psexec as our tool
                    QString cmd = QString("pkexec env DISPLAY=\"%1\" XAUTHORITY=\"%2\" %3")
                                  .arg(display, xauthority, QApplication::applicationFilePath());
                    system(cmd.toStdString().c_str());
                    return 0;
                }
                else if (ret == QMessageBox::Cancel)
                {
                    // okay, user cancel it.
                    return 0;
                }
            }

            // okay, we use sudo as our tool
            QMessageBox::information(NULL, QString("EasyDrcomGUI"), QString("请您在即将打开的终端界面中输入root密码，点击\"OK\"以继续。"));
            QString cmd = QString("gnome-terminal -e 'sudo \"%1\"'").arg(QApplication::applicationFilePath());
            system(cmd.toStdString().c_str());
            return 0;
        }*/

        QMessageBox::information(NULL, QString("EasyDrcomGUI"), QString("EasyDrcomGUI需要以root权限运行。\n\n"
                                                                        "在终端执行以下命令以授权：\n"
                                                                        "sudo \"%1\"").arg(QApplication::applicationFilePath()));
        return 0;
    }
}
