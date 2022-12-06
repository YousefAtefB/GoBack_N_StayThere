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

#include "Coordinator.h"
#include <fstream>
using namespace std;

Define_Module(Coordinator);

void Coordinator::initialize()
{
    // TODO - Generated method body
    ifstream input("coordinator.txt");
    int NodeId;
    double StartingTime;
    input>>NodeId>>StartingTime;
    cMessage* ToSender=new cMessage("Sender");
    ToSender->setTimestamp(StartingTime);
    cMessage* ToReceiver=new cMessage("Receiver");

    send(ToSender,NodeId==0?"out0":"out1");
    send(ToReceiver,NodeId==1?"out0":"out1");
}

void Coordinator::handleMessage(cMessage *msg)
{
    // TODO - Generated method body
}
