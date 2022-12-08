//
// Generated file, do not edit! Created by nedtool 5.6 from Frame.msg.
//

#ifndef __FRAME_M_H
#define __FRAME_M_H

#if defined(__clang__)
#  pragma clang diagnostic ignored "-Wreserved-id-macro"
#endif
#include <omnetpp.h>

// nedtool version check
#define MSGC_VERSION 0x0506
#if (MSGC_VERSION!=OMNETPP_VERSION)
#    error Version mismatch! Probably this file was generated by an earlier version of nedtool: 'make clean' should help.
#endif



/**
 * Class generated from <tt>Frame.msg:19</tt> by nedtool.
 * <pre>
 * //
 * // TODO generated message class
 * //
 * packet Frame
 * {
 *     \@customize(true);  // see the generated C++ header for more info
 *     int seq_num;
 *     int frame_type; // 0 data , 1 ack , 2 nack 
 *     int ack_num;   // 0 -> 1 -> 2 -> 
 *     string payload; // message  
 *     char parity_byte; // check 
 * }
 * </pre>
 *
 * Frame_Base is only useful if it gets subclassed, and Frame is derived from it.
 * The minimum code to be written for Frame is the following:
 *
 * <pre>
 * class Frame : public Frame_Base
 * {
 *   private:
 *     void copy(const Frame& other) { ... }

 *   public:
 *     Frame(const char *name=nullptr, short kind=0) : Frame_Base(name,kind) {}
 *     Frame(const Frame& other) : Frame_Base(other) {copy(other);}
 *     Frame& operator=(const Frame& other) {if (this==&other) return *this; Frame_Base::operator=(other); copy(other); return *this;}
 *     virtual Frame *dup() const override {return new Frame(*this);}
 *     // ADD CODE HERE to redefine and implement pure virtual functions from Frame_Base
 * };
 * </pre>
 *
 * The following should go into a .cc (.cpp) file:
 *
 * <pre>
 * Register_Class(Frame)
 * </pre>
 */
class Frame_Base : public ::omnetpp::cPacket
{
  protected:
    int seq_num;
    int frame_type;
    int ack_num;
    ::omnetpp::opp_string payload;
    char parity_byte;

  private:
    void copy(const Frame_Base& other);

  protected:
    // protected and unimplemented operator==(), to prevent accidental usage
    bool operator==(const Frame_Base&);

    // make assignment operator protected to force the user override it
    Frame_Base& operator=(const Frame_Base& other);

  public:
    // make constructors protected to avoid instantiation
    Frame_Base(const char *name=nullptr, short kind=0);
    Frame_Base(const Frame_Base& other);

    virtual ~Frame_Base();
    virtual Frame_Base *dup() const override {return new Frame_Base(*this);}
    virtual void parsimPack(omnetpp::cCommBuffer *b) const override;
    virtual void parsimUnpack(omnetpp::cCommBuffer *b) override;

    // field getter/setter methods
    virtual int getSeq_num() const;
    virtual void setSeq_num(int seq_num);
    virtual int getFrame_type() const;
    virtual void setFrame_type(int frame_type);
    virtual int getAck_num() const;
    virtual void setAck_num(int ack_num);
    virtual const char * getPayload() const;
    virtual void setPayload(const char * payload);
    virtual char getParity_byte() const;
    virtual void setParity_byte(char parity_byte);
};


#endif // ifndef __FRAME_M_H
