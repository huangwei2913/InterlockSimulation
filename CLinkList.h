#ifndef CLINKLIST_H
#define CLINKLIST_H
#include "LNode.h"

class CLinkList
{
public:
     CLinkList();
    ~CLinkList();

    int countNode;                  //节点个数计数（不包括头结点）
    LNode *head;                    //头结点指针
    void initCLinkList();						//创建链表初始化
    void insertLNode(int pos, QString elem);	//插入节点到链表
    void deleteLNode(int pos, QString &elem);	//删除链表节点
    bool locateLNode(int &pos,QString elem);	//查找链表节点
    QString getNodeDataByPosition(int pos);            //根据节点序号获取
    void destroySelf();							//释放申请的内存空间

};

#endif // CLINKLIST_H
