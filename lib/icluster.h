// interface for a cluster DLL
// in addition, each computing cluster DLL has to implement std::shared_ptr<ICluster> getCluster() to return cluster implementation

#ifndef INCLUDED_ICLUSTER_H
#define INCLUDED_ICLUSTER_H

#include "timebuff.h" // TBPtr
#include "main.h"     // ClusterID


struct IWriter {
    virtual int write(int nodeIndex, const Time& t) = 0;
    // virtual int read(const Time&); // TODO: implement instead of passing TBPtr to write()
};


class ICluster {
protected:
    ClusterID id;
public:
    ICluster(): id(0) { }
    virtual ClusterID getId(){ return id; }
    virtual void setId(ClusterID cid){ id = cid; } // stop if new id is 0?
    virtual std::string command(const char*)=0;
    virtual int read(IWriter&)=0;      // rename to poll() ???
    virtual int write(const TBPtr&)=0; // rename to input() ???
    virtual bool isWaitForInput(){ return false; } // block if there is nothing to write?
};


#endif // INCLUDED_ICLUSTER_H
