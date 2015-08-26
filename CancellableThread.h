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

#ifndef THREAD_H
#define THREAD_H
#include <QThread>
#include <QMessageBox>

class CancellableThread : public QThread
{
public:
    CancellableThread() : m_bCancelled(false) {}
    void cancel() { m_bCancelled = true; }
    void setAutoDelete() { connect(this, &QThread::finished, this, &CancellableThread::cleanup); }
    void cleanup() { delete this; }

protected:
    bool isCancelled() const { return m_bCancelled; }
    virtual void run() = 0;

private:
    bool m_bCancelled;
};

#define DECLARE_THREAD_OBJECT(theClass)				\
    template <typename T>							\
    class theClass : public CancellableThread {     \
    private:										\
        T* dlg;										\
        void* param;								\
    public:											\
        theClass(T* dlg, void* param) 				\
            : dlg(dlg), param(param) {}             \
        void run(); 								\
    };

#endif // THREAD_H
