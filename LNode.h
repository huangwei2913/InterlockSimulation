#ifndef LNODE_H
#define LNODE_H
#include<QString>

class LNode
{
public:
    LNode(QString dt, LNode *nt);       //构造函数
    ~LNode();

public:
    QString data;       //节点数据域
    LNode* next;       //节点指针域
};

#endif // LNODE_H
