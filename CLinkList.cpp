#include "CLinkList.h"

CLinkList::CLinkList()
{
    countNode=0;
    head=NULL;

}


void CLinkList::initCLinkList(){
    head=new LNode("head", NULL);
    head->next=head;
}


void CLinkList::insertLNode(int pos, QString elem){
    LNode *pInsertNode=NULL;
    LNode *pLNode=head;
    //找到前驱节点指针
    for(int i=0;i<pos-1;++i)
    {
        pLNode=pLNode->next;
    }

    //新节点插入到链表中
    pInsertNode=new LNode(elem,pLNode->next);
    pLNode->next=pInsertNode;
    ++countNode;

}	//插入节点到链表

//删除链表节点
void CLinkList::deleteLNode(int pos, QString &elem)
{
    LNode *pDeleteNode=NULL;
    LNode *pLNode=head;

    //找到前驱节点指针
    for(int i=0;i<pos-1;++i)
    {
        pLNode=pLNode->next;
    }
    pDeleteNode=pLNode->next;
    pLNode->next=pDeleteNode->next;
    elem=pDeleteNode->data;
    //删除节点，移除图形Item
    delete pDeleteNode;
    --countNode;

}

//获取指定位置节点的数据信息
QString CLinkList::getNodeDataByPosition(int pos){
    LNode *pLNode=head;
    int tmppos=0;
    while(pLNode->next!=head && tmppos++<pos){
        pLNode = pLNode->next;
    }
    return pLNode->data;
}

//查找链表节点
bool CLinkList::locateLNode(int &pos, QString elem)
{
    LNode *pLNode=head;
    for(pos=1;pLNode->next!=head&&pLNode->next->data!=elem;++pos)
    {
        pLNode=pLNode->next;
    }
    //找到相应节点
    if(pLNode->next!=head){
        return true;
    }
    return false;
}


//释放申请的内存空间
void CLinkList::destroySelf()
{

    LNode *pLNode=head->next, *qLNode;
    for(;pLNode!=head;pLNode=qLNode)
    {
        qLNode=pLNode->next;
        delete pLNode;      //释放内存
    }
    ////////////
    delete head;                //释放内存
    ///////////////
    countNode=0;
}
