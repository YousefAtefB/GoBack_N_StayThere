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
//    cout<<parity.to_ulong()<<endl;

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
    Frame_Base* Frame=new Frame_Base(*Frames[S]);

    ofstream output("output.txt",ios_base::app);
    if(SentBefore[S]==false)// only output this to the log for the first time to send the frame
    {
        SentBefore[S]=true;
        //-------------LOG
        output<<"At time ["<<simTime()-par("PT").doubleValue()<<"], Node["<<(string(getName())=="node0"?0:1)<<"]";
        output<<", Introducing channel error with code=["<<ErrorCode<<"]."<<endl;
        //-------------LOG
    }

    if(WithError==true&&ErrorCode[0]=='1')//Modification
    {
        //TODO: modify Message
        string PayLoad=Frame->getPayload();
        PayLoad[0]^=1;
        Frame->setPayload(PayLoad.c_str());
    }

    double ArrivalTime=par("TD").doubleValue();

    if(WithError==true&&ErrorCode[3]=='1')//Delay
    {
        ArrivalTime+=par("ED").doubleValue();
    }

    //send first
    if(WithError==false||ErrorCode[1]!='1')//No Loss in order to send
    {
        sendDelayed(Frame,ArrivalTime,"out");
    }

    string ParityByte;
    for(int i=7;i>=0;i--)
        ParityByte+=((Frame->getParity_byte()>>i)&1)?"1":"0";

    //-------------LOG
    output<<"At time ["<<simTime()<<"], Node["<<(string(getName())=="node0"?0:1)<<"]";
    output<<" [sent] frame with seq_num=["<<Frame->getSeq_num()<<"] "<<"and payload=["<<Frame->getPayload()<<"] and trailer=[";
    output<<ParityByte<<"] , Modified ["<<(WithError==true&&ErrorCode[0]=='1'?1:-1)<<"] ,Lost ["<<(WithError==true&&ErrorCode[1]=='1'?"YES":"NO")<<"]";
    output<<", Duplicate ["<<(WithError==true&&ErrorCode[2]=='1'?1:0)<<"], Delay["<<(WithError==true&&ErrorCode[3]=='1'?par("ED").doubleValue():0)<<"]."<<endl;
    //-------------LOG

    if(WithError==true&&ErrorCode[2]=='1')//Duplication
    {
        ArrivalTime+=par("DD").doubleValue();
        //send second
        Frame_Base* Frame2=new Frame_Base(*Frame);
        if(ErrorCode[1]!='1')//No Loss in order to send
            sendDelayed(Frame2,ArrivalTime,"out");

        //-------------LOG
        output<<"At time ["<<simTime()+par("DD").doubleValue()<<"], Node["<<(string(getName())=="node0"?0:1)<<"]";
        output<<" [sent] frame with seq_num=["<<Frame->getSeq_num()<<"] "<<"and payload=["<<Frame->getPayload()<<"] and trailer=[";
        output<<ParityByte<<"] , Modified ["<<(ErrorCode[0]=='1'?1:-1)<<"] ,Lost ["<<(ErrorCode[1]=='1'?"YES":"NO")<<"]";
        output<<", Duplicate ["<<2<<"], Delay["<<(ErrorCode[3]=='1'?par("ED").doubleValue():0)<<"]."<<endl;
        //-------------LOG
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
        cnt=(cnt+1)%(par("WS").intValue());
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
        //SentBefore keeps track of the frame that have been sent before used to output the log file
        SentBefore=vector<bool>(Frames.size(),false);
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
    else if(Sender==true)
    {
        if(msg->isSelfMessage())//send another message from sender after Starting time or TIMEOUT
        {
            if(string(msg->getName())=="Start Session")// start of session begin to send the first window
            {
                cMessage* Msg=new cMessage("Send Frame");
                //kind means with error or not
                Msg->setKind(1);
                //schedule to send first frame
                scheduleAt(simTime()+par("PT").doubleValue(),Msg);
            }
            else if(string(msg->getName())=="Send Frame")//it is time to send a frame after processing time
            {
                EV<<"before sent S SF SL: "<<S<<" "<<SF<<" "<<SL<<endl;
                //not sure we need this line but runtime without it
                if(S>=Frames.size())return;
                //we send the frame with error if its the start of the session
                //or if it is a timer but not the frame that caused timing out
                //I THINK THAT SF IS THE ONE THAT WILL ALWAYS CAUSE THE TIMEOUT
                // kind = 1 means send with error
                EV<<"before send frame:"<<simTime()<<"\n";
                SendFrame(msg->getKind()==1);
                //schedule a new timer and send the frame number and timer number
                string str=to_string(S)+" "+to_string(TimerNumber[S]);
                cMessage *TimeOutMsg=new cMessage(str.c_str());
                scheduleAt(simTime()+par("TO").doubleValue(), TimeOutMsg);
                //increase S
                S++;
                //if the window is not finished schedule for next frame
                if(S<=SL)
                {
                    cMessage* Msg=new cMessage("Send Frame");
                    //send with error, kind means with error or not
                    Msg->setKind(1);
                    //schedule to send first frame
                    scheduleAt(simTime()+par("PT").doubleValue(),Msg);
                }
            }
            else //Time Out send the window again but first frame without errors
            {
                //if it is a schedule for a timer we need to check that the timer is not refreshed
                //using TimerNumber and the frame that timedout is not acknowledged

                //frame that caused timeout
                stringstream ss(msg->getName());
                int Frame,Timer;
                ss>>Frame>>Timer;
                //ignore timer if frame already acknowledged or timer is old (refreshed)
                if(Frame<SF||Timer!=TimerNumber[Frame])return;
                //reset S
                S=SF;
                //send the whole window after timeout
                //we send the frame with error if its the start of the session
                //or if it is a timer but not the frame that caused timing out
                //I THINK THAT SF IS THE ONE THAT WILL ALWAYS CAUSE THE TIMEOUT

                //schedule for first frame in the window without errors
                cMessage* Msg=new cMessage("Send Frame");
                //send without error, kind means with error or not
                Msg->setKind(0);
                //schedule to send first frame
                scheduleAt(simTime()+par("PT").doubleValue(),Msg);

                //refresh the timer by increasing its number
                for(int i=SF;i<=SL&&i<Frames.size();i++)
                    TimerNumber[i]++;

                //-------------LOG
                ofstream output("output.txt",ios_base::app);
                output<<"Time out event at time ["<<simTime()<<"], "<<"at Node["<<(string(getName())=="node0"?0:1);
                output<<"] for frame with seq_num=["<<Frames[Frame]->getSeq_num()<<"]"<<endl;
                //-------------LOG
            }
        }
        else//handle ACK & NACK from sender
        {
            Frame_Base* Frame=check_and_cast<Frame_Base*>(msg);
            //ignore NACK
            if(Frame->getFrame_type()==2)return;

            //get AckNum of the ACK
            int AckNum=Frame->getAck_num();

            //the orignal window is finished we need to schedule for the new ack frames
            if(S>SL)
            {
                cMessage* Msg=new cMessage("Send Frame");
                //send with error, kind means with error or not
                Msg->setKind(1);
                //schedule to send first frame
                scheduleAt(simTime()+par("PT").doubleValue(),Msg);
            }
            //0 1 2 3 4 5

            //shift SF & SL
            //0 1 2 3 4 5
            //0 1 2 3 4 5 0 1 2 3 4  5 0 1 2
            //0 1 2 3 4 5 6 7 8 9 10 11
            int WS=par("WS").intValue();
            int shift=(AckNum-(SF%WS)+WS)%(WS);
            SF+=shift;
            SL+=shift;

            EV<<"after ack S SF SL: "<<S<<" "<<SF<<" "<<SL<<endl;
        }
    }
    else if(Sender==false)//Send ACK from receiver
    {
        //delay time for ack/nack is just TD
        double ArrivalTime=par("PT").doubleValue()+par("TD").doubleValue();

        Frame_Base* Frame=check_and_cast<Frame_Base*>(msg);
        Frame_Base* SendMsg=new Frame_Base(" ");

        if(R!=Frame->getSeq_num()) // if out of order frame just ignore it
            return;

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
                R=(R+1)%(par("WS").intValue());
                SendMsg->setFrame_type(1);
                SendMsg->setAck_num(R);
                EV<<"before send ack:"<<simTime()+par("PT").doubleValue()<<"\n";
                sendDelayed(SendMsg,ArrivalTime,"out");
            }
//            else
//            {
//                //Unexpected Frame
//                SendMsg->setFrame_type(2);
//                SendMsg->setAck_num(R);
//                sendDelayed(SendMsg,ArrivalTime,"out");
//            }
        }

        //-------------LOG
        ofstream output("output2.txt",ios_base::app);
        output<<"At time["<<simTime()+par("PT").doubleValue()<<"], Node["<<(string(getName())=="node0"?0:1);
        output<<"] Sending ["<<(SendMsg->getFrame_type()==1?"ACK":"NACK")<<"] with number ["<<SendMsg->getAck_num()<<"] , loss [";
        output<<(rand<100-par("LP").intValue()?"NO":"YES")<<"]"<<endl;
        //-------------LOG


        EV<<simTime()<<" "<<Frame->getPayload()<<endl;
    }

}
