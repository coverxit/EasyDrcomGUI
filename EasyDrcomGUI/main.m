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

#import <Cocoa/Cocoa.h>
#import <sys/file.h>
#import <errno.h>

int main(int argc, const char * argv[])
{
    int lck_file = open("/tmp/EasyDrcomGUI.lck", O_CREAT | O_RDWR, 0666);
    int rc = flock(lck_file, LOCK_EX | LOCK_NB);
    if (!rc) // first instance
        return NSApplicationMain(argc, argv);
    else
    {
        if (EWOULDBLOCK == errno) // there is already an instance running
            return 0;
        else // other error, but we don't care
            return errno;
    }
}
