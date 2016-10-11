#pragma once

struct expr_node
{
  COND      *left_op;
  COND      *operation;
  COND      *right_op;
  COND      *junction;
  expr_node *next;
};

class Expression
{
public:
  Expression(void);
  int remove_expression(int num, bool free);
  expr_node *get_expression(int num);
  int add_expression(bool append, expr_node *new_item);
  int num_expressions();
  int index_of(char *table, char *value);
  int reduce_expressions(TABLE *table);
  bool has_table(char *table);
  int convert(THD *thd, COND *mysql_expr);
  char *to_string();
  bool evaluate(TABLE *table1);
  int compare_join(expr_node *expr, TABLE *t1, TABLE *t2);
  int get_join_expr(Expression *where_expr);
private:
  expr_node *root;
  Field *find_field(TABLE *tbl, char *name);
  bool compare(expr_node *expr, TABLE *t1);
  int num_expr;
};
