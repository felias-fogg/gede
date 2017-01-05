/*
 * Copyright (C) 2014-2017 Johan Henriksson.
 * All rights reserved.
 *
 * This software may be modified and distributed under the terms
 * of the BSD license.  See the LICENSE file for details.
 */

#ifndef FILE__TREE_H
#define FILE__TREE_H

#include <QString>
#include <QVector>
#include <QList>
#include <QStringList>
#include <stdint.h>


class TreeNode
{
public:
    TreeNode();
    virtual ~TreeNode();
    
    TreeNode *findChild(QString path) const;

    uint32_t getAddress() const { return m_address; };
    void setAddress(uint32_t addr) { m_address = addr; };
    
    QStringList getChildList() const;
    void addChild(TreeNode *child) { m_children.push_back(child); };
    TreeNode *getChild(int i) const { return m_children[i]; };
    int getChildCount() const { return m_children.size(); };
    QString getData() const { return m_data; };
    void setData(QString data) { m_data = data; };
    void dump();

    void setName(QString name) { m_name = name; };
    QString getName() const { return m_name; };

    void removeAll();


    void copy(const TreeNode &other);
private:
    void dump(int parentCnt);


public:
    QString m_name;
    QString m_data;
    QVector<TreeNode*> m_children;
    uint32_t m_address;

private:
        TreeNode(const TreeNode &) { };

};


class Token;


class Tree
{
public:
    Tree();
    


    void dump() { m_root.dump();};

    
    QString getString(QString path) const;
    int getInt(QString path, int defaultValue = 0) const;
    long long getLongLong(QString path) const;

    TreeNode *getChildAt(int idx) { return m_root.getChild(idx);};
    int getRootChildCount() const { return m_root.getChildCount();};
    int getChildCount(QString path) const;
    QStringList getChildList(QString path) const;
    

    TreeNode* getRoot() { return &m_root; };
    void copy(const Tree &other);

    void removeAll();
    
private:

    TreeNode fromStringToTree(QString str);
    QList<Token*> tokenize(QString str);
    Token* pop_token();
    Token* peek_token();
    
    void parseGroup(TreeNode *parentNode);
    void parseVar(TreeNode *parentNode);
    void parseVarList(TreeNode *parentNode);
    void parseData(TreeNode *ownerNode);
    void parseDataList(TreeNode *ownerNode);

private:
    Tree(const Tree &) {}; 
private:

    TreeNode m_root;
};

#endif // FILE__TREE_H
