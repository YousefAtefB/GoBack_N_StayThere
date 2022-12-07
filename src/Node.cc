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

void Node::SendFrame(bool WithError)
{
    string ErrorCode=Frames[S].substr(0,4);
    string Message=Frames[S].substr(4);

    if(WithError==true && ErrorCode[1]=='1')//Loss
    {
        return;
    }

    if(WithError==true && ErrorCode[0]=='1')//Modification
    {
        //TODO: modify Message
    }

    double ArrivalTime=par("PT").doubleValue()*(SF-S)+par("TD").doubleValue();

    if(WithError==true && ErrorCode[3]=='1')//Delay
    {
        ArrivalTime+=par("ED").doubleValue();
    }

    //send first
    sendDelayed(new cMessage(Message.c_str()),ArrivalTime,"out");
    if(WithError==true && ErrorCode[2]=='1')//Duplication
    {
        ArrivalTime+=par("DD").doubleValue();
        //send second
        sendDelayed(new cMessage(Message.c_str()),ArrivalTime,"out");
    }
}

void Node::handleMessage(cMessage *msg)
{
    if(string(msg->getName())=="Sender")//handle coordinator if sender
    {
        //mark as sender
        Sender=true;
        //open corresponding file
        string FileName=string(getName())=="node0"?"input0.txt":"input1.txt";
        ifstream input(FileName);
        //set window from 0 to WS-1
        S=0,SF=0,SL=par("WS").intValue()-1;
        //read all frames from the file
        string str;
        while(getline(input,str))
            Frames.push_back(str);
        //TimerNumber keeps track of the timer for each frame in order to ignore Timeouts when we refresh the timers
        TimerNumber=vector<int>(Frames.size(),0);
        //schedule the starting message
        scheduleAt(msg->getTimestamp(),new cMessage("Start Session"));
    }
    else if(string(msg->getName())=="Receiver")//handle coordinator if receiver
    {
        //mark it as receiver
        Sender=false;
        //set expected frame to 0
        R=0;
    }
    else if(msg->isSelfMessage())//send another message from sender after Starting time or TIMEOUT
    {
        //check if staring session message
        bool StartSession=string(msg->getName())=="Start Session";
        //if it is a schedule for a timer we need to check that the timer is not refreshed
        //using TimerNumber and the frame that timedout is not acknowledged
        if(StartSession==false)
        {
            //frame that caused timeout
            stringstream ss(msg->getName());
            int Frame,Timer;
            ss>>Frame>>Timer;
            //ignore timer if frame already acknowledged or timer is old (refreshed)
            if(Frame<SF||Timer!=TimerNumber[Frame])return;
        }
        //reset S
        S=SF;
        //send the whole window after timeout
        while(S<=SL&&S<Frames.size())
        {
            //we send the frame with error if its the start of the session
            //or if it is a timer but not the frame that caused timing out
            //I THINK THAT SF IS THE ONE THAT WILL ALWAYS CAUSE THE TIMEOUT
            SendFrame(StartSession==true || S!=SF);
            //refresh the timer by increasing its number
            TimerNumber[S]++;
            //schedule a new timer and send the frame number and timer number
            string str=to_string(S)+" "+to_string(TimerNumber[S]);
            cMessage *TimeOutMsg=new cMessage(str.c_str());
            scheduleAt(simTime()+par("TO").doubleValue(), TimeOutMsg);
            //increase S
            S++;
        }
    }
    else if(Sender==true)//handle ACK & NACK from sender
    {
        if(string(msg->getName())=="NACK")return;
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
    else if(Sender==false)//Send ACK from receiver
    {
        //TODO: if error dedicted send NACK
        int rand=uniform(0,1)*100;
        //Loss probability
        if(rand<100-par("LP").intValue())
        {
            //delay time for ack/nack is just TD
            double ArrivalTime=par("PT").intValue()+par("TD").intValue();
            //Expected Frame
            if(R==atoi(msg->getName()))//seqnum
            {
                R=(R+1)%(par("WS").intValue()+1);
                sendDelayed(new cMessage(("ACK "+to_string(R)).c_str()),ArrivalTime,"out");
            }
            else
            {
                //Unexpected Frame
                sendDelayed(new cMessage("NACK"),ArrivalTime,"out");
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
