//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
// 
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see http://www.gnu.org/licenses/.
// 

#ifndef __PROJECT_NODE_H_
#define __PROJECT_NODE_H_

#include <omnetpp.h>
#include <vector>
#include <string>
#include <fstream>
#include <bitset>
#include "Frame_m.h"
using namespace std;

using namespace omnetpp;

/**
 * TODO - Generated class
 */
class Node : public cSimpleModule
{
  protected:
    bool Sender;
    int SF,S,SL;
    int R;
    vector<int>TimerNumber;
    vector<bool>SentBefore;
    vector<Frame_Base*>Frames;
    vector<string>ErrorCodes;
    virtual void CreateFrames();
    virtual void SendWindow(bool WithError);
    virtual void SendFrame(bool WithError);
    virtual void initialize();
    virtual void handleMessage(cMessage *msg);
};

#endif
