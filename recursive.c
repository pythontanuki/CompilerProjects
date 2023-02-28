#include "9cc.c"
/*Write Code of Recursive Parser Code.*/
/*
expr = mul("+" mul | "-" mul")*
  mul = primary("*" primary | "/" primary)*
  primary = num | "("expr")"
*/

/*Struct of AST*/
typedef enum {
    ND_ADD,
    ND_SUB,
    ND_MUL,
    ND_DIV,
    ND_NUM,
} NodeKind;

typedef struct Node Node;
Node* expr();
Node* primary();
Node* mul();

struct Node {
    NodeKind kind;//type of node
    Node *lhs;//left_child
    Node *rhs;//right_child
    int val;//value of node, if kind is ND_NUM.
};

/*Write Function to make new Node.*/

/*newNode makes the new Node whose type is excepted for ND_NUM.*/
Node* newNode(NodeKind kind, Node *lhs, Node *rhs) {
    Node *node = calloc(1,sizeof(Node));
    node->kind = kind;
    node->lhs = lhs;
    node->rhs = rhs;
    return node;
}

/*newNode_num makes the new Node whose type is ND_NUM, only.*/
Node* newNode_num(int val) {
    Node *node_num = calloc(1,sizeof(Node));
    node_num->kind = ND_NUM;
    node_num->val = val;
    return node_num;
}

/*These are LL(1) Parsa methods.*/
Node* expr() {
    Node *node = mul();
    for(;;) {
        if(consume('+')) node = newNode(ND_ADD, node, mul());
        else if(consume('-')) node = newNode(ND_SUB, node, mul());
        else return node;
    }
}


Node* mul() {
    Node *node = primary();
    for(;;) {
        if(consume('*')) node = newNode(ND_MUL, node, primary());
        else if(consume('/')) node = newNode(ND_DIV, node, primary());
        else return node;
    }
}

Node* primary() {
    if(consume('(')) {
        Node *node = expr();
        expect(')');
        return node;
    } 
    return newNode_num(expect_number());
}

