#include <ctype.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// トークンの種類
typedef enum {
  TK_RESERVED, // 記号
  TK_NUM,      // 整数トークン
  TK_EOF,      // 入力の終わりを表すトークン
} TokenKind;

typedef struct Token Token;

// トークン型
struct Token {
  TokenKind kind; // トークンの型
  Token *next;    // 次の入力トークン
  int val;        // kindがTK_NUMの場合、その数値
  char *str;      // トークン文字列
};

// 現在着目しているトークン
Token *token;
char *user_input;

// エラーを報告するための関数
// printfと同じ引数を取る
void error(char *fmt, ...) {
  va_list ap;
  va_start(ap, fmt);
  vfprintf(stderr, fmt, ap);
  fprintf(stderr, "\n");
  exit(1);
}

/*可変長引数にダイナミズムを持たせるにはva_listとして、新しい１つの変数にすることが大事*/

void error_at(char *loc, char *fmt, ...) {
    va_list ap;
    /*可変長引数を１つの変数にまとめている。可変長引数の先頭のアドレスだけ渡す。
    そこからは型を見て、オフセットで管理する。*/
    va_start(ap,fmt);
    int pos = loc-user_input;
    fprintf(stderr, "%s\n", user_input);
    fprintf(stderr, "%*s", pos, " ");
    fprintf(stderr, "^");
    //vprintf(stderr, fmt, ap)
    //まとめられた変数で処理する。
    vprintf(fmt, ap);
    fprintf(stderr, "\n");
    exit(1);
}

// 次のトークンが期待している記号のときには、トークンを1つ読み進めて
// 真を返す。それ以外の場合には偽を返す。
bool consume(char op) {
  if (token->kind != TK_RESERVED || token->str[0] != op)
    return false;
  token = token->next;
  return true;
}

// 次のトークンが期待している記号のときには、トークンを1つ読み進める。
// それ以外の場合にはエラーを報告する。
void expect(char op) {
  if (token->kind != TK_RESERVED || token->str[0] != op)
    error_at(token->str, "'%c'ではありません\n", op);
  token = token->next;
}

// 次のトークンが数値の場合、トークンを1つ読み進めてその数値を返す。
// それ以外の場合にはエラーを報告する。
int expect_number() {
  if (token->kind != TK_NUM)
    error_at(token->str, "数ではありません\n");
  int val = token->val;
  token = token->next;
  return val;
}

bool at_eof() {
  return token->kind == TK_EOF;
}

// 新しいトークンを作成してcurに繋げる
Token *new_token(TokenKind kind, Token *cur, char *str) {
  Token *tok = calloc(1, sizeof(Token));
  tok->kind = kind;
  tok->str = str;
//   printf("%s\n", tok->str);
  cur->next = tok;
  return tok;
}

// 入力文字列pをトークナイズしてそれを返す
Token *tokenize() {
  char *p = user_input;
  Token head;
  head.next = NULL;
  Token *cur = &head;

  while (*p) {
    // printf("%c\n", *p);
    // 空白文字をスキップ
    if (isspace(*p)) {
      p++;
      continue;
    }

    if (strchr("+-*/()", *p)) {
      cur = new_token(TK_RESERVED, cur, p++);
      continue;
    }

    if (isdigit(*p)) {
      cur = new_token(TK_NUM, cur, p);
    //   printf("%s\n", p);
    //   printf("%c\n", *p);
      cur->val = strtol(p, &p, 10);
    //   printf("%d\n", cur->val);
      continue;
    }
    // puts("ok");
    error_at(p,"トークナイズできません\n");
  }

  new_token(TK_EOF, cur, p);
  return head.next;
}

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
Node *expr();
Node *primary();
Node *mul();

struct Node {
    NodeKind kind;//type of node
    Node *lhs;//left_child
    Node *rhs;//right_child
    int val;//value of node, if kind is ND_NUM.
};

/*Write Function to make new Node.*/

/*newNode makes the new Node whose type is excepted for ND_NUM.*/
Node *newNode(NodeKind kind, Node *lhs, Node *rhs) {
    Node *node = calloc(1,sizeof(Node));
    node->kind = kind;
    node->lhs = lhs;
    node->rhs = rhs;
    return node;
}

/*newNode_num makes the new Node whose type is ND_NUM, only.*/
Node *newNode_num(int val) {
    Node *node_num = calloc(1,sizeof(Node));
    node_num->kind = ND_NUM;
    node_num->val = val;
    return node_num;
}

/*These are LL(1) Parsa methods.*/
Node *expr() {
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

void gen(Node *node) {
  if (node->kind == ND_NUM) {
    printf("  push %d\n", node->val);
    return;
  }

  gen(node->lhs);
  gen(node->rhs);

  printf("  pop rdi\n");
  printf("  pop rax\n");

  switch (node->kind) {
  case ND_ADD:
    printf("  add rax, rdi\n");
    break;
  case ND_SUB:
    printf("  sub rax, rdi\n");
    break;
  case ND_MUL:
    printf("  imul rax, rdi\n");
    break;
  case ND_DIV:
    printf("  cqo\n");
    printf("  idiv rdi\n");
    break;
  }

  printf("  push rax\n");
}

int main(int argc, char **argv) {
  if (argc != 2) {
    error("引数の個数が正しくありません");
    return 1;
  }
  // トークナイズする
  user_input = argv[1];
  token = tokenize();
  Node *node = expr();

  // アセンブリの前半部分を出力
  printf(".intel_syntax noprefix\n");
  printf(".globl main\n");
  printf("main:\n");

  // 抽象構文木を下りながらコード生成
  gen(node);

  // スタックトップに式全体の値が残っているはずなので
  // それをRAXにロードして関数からの返り値とする
  printf("  pop rax\n");
  printf("  ret\n");
  return 0;
}