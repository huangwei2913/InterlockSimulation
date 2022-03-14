#include "LNode.h"
LNode::LNode(QString dt, LNode *nt)
{
    data=dt;
    next=nt;
}

LNode::~LNode()
{

}
