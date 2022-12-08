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

string byte_stuffing(string str )
{
    int i = 0 ;
    while(i< str.size()){
         if(str[i]=='$' || str[i]=='/'){
                str.insert(i,"/");
                i=i+2;
                continue;
        }
        i++;
    }
    str.append("$");
    str.insert(0,"$");

    return str ;
}

char get_parity_byte(string str )
{
    int i =0 ;
    vector< std::bitset<8> > vec;
    for (i=0 ; i<str.size(); i++){
        vec.push_back(bitset<8>(str[i]));
    }

    bitset<8> parity(0);
    for (i=0; i<vec.size();i++){
        parity=(parity ^ vec[i]);
    }
//        vec.push_back(parity);
    cout<<parity.to_ulong()<<endl;

    return (char)parity.to_ulong();
}

string get_from_byte_stuffing(string  str )
{

    string recieved =str ;
    recieved.pop_back();// removing the $ end of the string
    recieved.erase(0,1);// removing the first char $
    int i=0;
   while(i< recieved.size()){
       if(recieved[i]=='/'){
           recieved.erase(i,1);
       }
       i++;
   }
   return recieved ;
}

void Node::SendFrame(bool WithError)
{
    string ErrorCode=ErrorCodes[S];
    Frame_Base* Frame=new Frame_Base(Frames[S]);

    if(WithError==true && ErrorCode[1]=='1')//Loss
    {
        return;
    }

    if(WithError==true && ErrorCode[0]=='1')//Modification
    {
        //TODO: modify Message
        string PayLoad=Frame->getPayload();
        PayLoad[0]^=1;
        Frame->setPayload(PayLoad);
    }

    double ArrivalTime=par("PT").doubleValue()*(S-SF+1)+par("TD").doubleValue();

    if(WithError==true && ErrorCode[3]=='1')//Delay
    {
        ArrivalTime+=par("ED").doubleValue();
    }

    //send first
    sendDelayed(Frame,ArrivalTime,"out");
    if(WithError==true && ErrorCode[2]=='1')//Duplication
    {
        ArrivalTime+=par("DD").doubleValue();
        //send second
        sendDelayed(Frame,ArrivalTime,"out");
    }
}

void Node::SendWindow(bool WithError)
{
    //send the whole window after timeout
    while(S<=SL&&S<Frames.size())
    {
        //we send the frame with error if its the start of the session
        //or if it is a timer but not the frame that caused timing out
        //I THINK THAT SF IS THE ONE THAT WILL ALWAYS CAUSE THE TIMEOUT
        SendFrame(WithError);
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

void Node::CreateFrames()
{
    //open corresponding file
    string FileName=string(getName())=="node0"?"input0.txt":"input1.txt";
    ifstream input(FileName);

    //read all frames from the file
    string str;
    int cnt=0;
    while(getline(input,str))
    {
        string ErrorCode=str.substr(0,4);
        string Message=str.substr(5);

        ErrorCodes.push_back(ErrorCode);

        Frame_Base* Frame=new Frame_Base(" ");
        Message=byte_stuffing(Message);
        Frame->setPayload(Message.c_str());
        char ParityByte=get_parity_byte(Message);
        Frame->setParity_byte(ParityByte);
        Frame->setSeq_num(cnt);
        Frame->setFrame_type(0);
        Frames.push_back(Frame);
        cnt=(cnt+1)%(par("WS").intValue()+1);
    }
}

void Node::handleMessage(cMessage *msg)
{
    if(string(msg->getName())=="Sender")//handle coordinator if sender
    {
        //mark as sender
        Sender=true;
        //set window from 0 to WS-1
        S=0,SF=0,SL=par("WS").intValue()-1;
        //Create frames
        CreateFrames();
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
        bool WithError=StartSession==true || S!=SF;
        //we send the frame with error if its the start of the session
        //or if it is a timer but not the frame that caused timing out
        //I THINK THAT SF IS THE ONE THAT WILL ALWAYS CAUSE THE TIMEOUT
        SendWindow(WithError);
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
        SendWindow(true);
    }
    else if(Sender==false)//Send ACK from receiver
    {
        EV<<simTime()<<" "<<msg->getName()<<endl;

        //delay time for ack/nack is just TD
        double ArrivalTime=par("PT").doubleValue()+par("TD").doubleValue();

        Frame_Base* Frame=check_and_cast<Frame_Base*>(msg);
        Frame_Base* SendMsg=new Frame_Base(" ");

        int rand=uniform(0,1)*100;
        //Loss probability
        if(rand<100-par("LP").intValue())
        {
            //if error dedicted send NACK
            if(Frame->getParity_byte()!=get_parity_byte(Frame->getPayload()))
            {
                SendMsg->setFrame_type(2);
                SendMsg->setAck_num(R);
                sendDelayed(SendMsg,ArrivalTime,"out");
            }
            else if(R==Frame->getSeq_num())
            {
                //Expected Frame
                R=(R+1)%(par("WS").intValue()+1);
                SendMsg->setFrame_type(1);
                SendMsg->setAck_num(R);
                sendDelayed(SendMsg,ArrivalTime,"out");
            }
            else
            {
                //Unexpected Frame
                SendMsg->setFrame_type(2);
                SendMsg->setAck_num(R);
                sendDelayed(SendMsg,ArrivalTime,"out");
            }
        }
    }

//    if(Sender==false)
//    {
//        EV<<simTime()<<" "<<msg->getName()<<endl;
//    }

//    EV<<getName()<<" "<<par("PT").doubleValue()<<endl;
//    EV<<Sender<<" "<<StartingTime<<endl;
}
