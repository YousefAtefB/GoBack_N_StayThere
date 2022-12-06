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

#include "Node.h"
#include <string>
using namespace std;

Define_Module(Node);

void Node::initialize()
{
    // TODO - Generated method body
}

void Node::handleMessage(cMessage *msg)
{
    // TODO - Generated method body
    if(string(msg->getName())=="Sender")
    {
        Sender=true;
        StartingTime=msg->getTimestamp();
    }

    if(string(msg->getName())=="Receiver")
    {
        Sender=false;
    }

    // if(Sender==true)
    // {
    //     ifstream input(string(getName())=="node0"?"input0.txt":"input1.txt");

    // }

    EV<<Sender<<" "<<StartingTime<<endl;
}
