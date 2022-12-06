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

Define_Module(Node);

void Node::initialize()
{
    // TODO - Generated method body
}

void Node::SendFrame()
{
    string ErrorCode=Frames[S].substr(0,4);
    string Message=Frames[S].substr(4);

    if(ErrorCode[1]=='1')//Loss
    {
        return;
    }

    if(ErrorCode[0]=='1')//Modification
    {
        //TODO: modify Message
    }

    double ArrivalTime=par("PT").doubleValue()+par("TD").doubleValue();

    if(ErrorCode[3]=='1')//Delay
    {
        ArrivalTime+=par("ED").doubleValue();
    }

    //send first
    sendDelayed(new cMessage(Message.c_str()),ArrivalTime,"out");
    if(ErrorCode[2]=='1')//Duplication
    {
        ArrivalTime+=par("DD").doubleValue();
        //send second
        sendDelayed(new cMessage(Message.c_str()),ArrivalTime,"out");
    }
}

void Node::handleMessage(cMessage *msg)
{
    // TODO - Generated method body
    if(string(msg->getName())=="Sender")
    {
        Sender=true;
        string FileName=string(getName())=="node0"?"input0.txt":"input1.txt";
        ifstream input(FileName);
        S=0,SF=0,SL=par("WS").intValue()-1;
        string str;
        while(getline(input,str))
            Frames.push_back(str);
        scheduleAt(msg->getTimestamp(),new cMessage(""));
    }
    else if(string(msg->getName())=="Receiver")
    {
        Sender=false;
        R=0;
    }
    else if(msg->isSelfMessage())
    {
        //window is not finished
        if(S<=SL&&S<Frames.size())
        {
            SendFrame();
            S++;
        }

        //if we sent the whole window
        if(S>SL)
        {
            S=SF;
            //Timeout
        }
        //schedule for the next frame to send after processing time
        scheduleAt(simTime()+par("PT"),new cMessage(""));
    }
    else if(Sender==true)//TODO: handle ACK & NACK
    {
        if(string(msg)=="NACK")return;
        //get seqnum of the ACK
        int seqnum=atoi(string(msg->getName()).substr(4).c_str());
        //shift SF & SL
        //0 1 2 3 4 5
        //0 1 2 3 4 5 0 1 2 3 4  5
        //0 1 2 3 4 5 6 7 8 9 10 11
        int WS=par("WS").intValue();
        int shift=(seqnum-SF+WS+1)%(WS+1);
        SF+=shift;
        SL+=shift;
    }
    else if(Sender==false)//TODO: send ACK
    {
        //TODO: if error dedicted send NACK
        int rand=uniform(0,1)*100;
        //Loss probability
        if(rand<100-par("LP").intValue())
        {
            //Expected Frame
            if(R==atoi(msg->getName()))//seqnum
            {
                R=(R+1)%(par("WS").intValue()+1);
                send(new cMessage("ACK "+to_string(R)),"out");
            }
            else
            {
                //Unexpected Frame
                send(new cMessage("NACK"),"out");
            }
        }
    }

    if(Sender==false)
    {
        EV<<simTime()<<" "<<msg->getName()<<endl;
    }

//    EV<<getName()<<" "<<par("PT").doubleValue()<<endl;
//    EV<<Sender<<" "<<StartingTime<<endl;
}
