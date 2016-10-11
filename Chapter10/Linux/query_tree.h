/*
  Query_tree.h

  DESCRIPTION
    This file contains the Query_tree class. It is responsible for containing the
    internal representation of the query to be executed. It provides methods for
    optimizing and forming and inspecting the query tree. This class is the very
    heart of the DBXP query capability! It also provides the ability to store
    a binary "compiled" form of the query.

  NOTES
    The data structure is a binary tree that can have 0, 1, or 2 children. Only
    Join operations can have 2 children. All other operations have 0 or 1 
    children. Each node in the tree is an operation and the links to children
    are the pipeline.
 
  SEE ALSO
    query_tree.cc
*/
#include "mysql_priv.h"

class Query_tree
{
public:
  enum query_node_type          //this enumeration lists the available
  {                              //query node (operations)
    qntUndefined = 0,
    qntRestrict = 1,
    qntProject = 2,
    qntJoin = 3,
    qntSort = 4,
    qntDistinct = 5
  };

  enum join_con_type            //this enumeration lists the available
  {                              //join operations supported
    jcUN = 0,
    jcNA = 1,
    jcON = 2,
    jcUS = 3
  };

  enum type_join                //this enumeration lists the available
  {                              //join types supported.
    jnUNKNOWN      = 0,          //undefined
    jnINNER        = 1,
    jnLEFTOUTER    = 2,
    jnRIGHTOUTER   = 3,
    jnFULLOUTER    = 4,
    jnCROSSPRODUCT = 5,
    jnUNION        = 6,
    jnINTERSECT    = 7
  };

    enum AggregateType          //used to add aggregate functions
    {
        atNONE      = 0,
        atCOUNT     = 1
    };

  /*
    STRUCTURE query_node

    DESCRIPTION
      This this structure contains all of the data for a query node:

      NodeId -- the internal id number for a node
      ParentNodeId -- the internal id for the parent node (used for insert)
      SubQuery -- is this the start of a subquery?
      Child -- is this a Left or Right child of the parent?
      NodeType -- synonymous with operation type
      JoinType -- if a join, this is the join operation
      join_con_type -- if this is a join, this is the "on" condition
      Expressions -- the expressions from the "where" clause for this node
      Join Expressions -- the join expressions from the "join" clause(s) 
      Relations[] -- the relations for this operation (at most 2)
      PreemptPipeline -- does the pipeline need to be halted for a sort?
      Fields -- the attributes for the result set of this operation
      Left -- a pointer to the left child node
      Right -- a pointer to the right child node
*/
  struct query_node            
  {
    query_node();
    //query_node(const query_node &o);
    ~query_node();
    int                 nodeid;
    int                 parent_nodeid;
    bool                sub_query;
    bool                child;
    query_node_type     node_type;
    type_join           join_type;
    join_con_type       join_cond;
    COND                *where_expr;
    COND                *join_expr;
    TABLE_LIST          *relations[4];
    bool                preempt_pipeline;
    List<Item>          *fields;
    query_node          *left;
    query_node          *right;
  };

  query_node *root;              //The ROOT node of the tree

  ~Query_tree(void);
  void ShowPlan(query_node *QN, bool PrintOnRight);

/*
  //**************************************************************************
  // ReadTree (public) 
  //**************************************************************************
  // This method reads a compiled query from a file (passed) and builds the
  // internal tree for it.
  //**************************************************************************
  //bool ReadTree(char *fn);

  //**************************************************************************
  // PrintQuery_tree (public) 
  //**************************************************************************
  // This method prints to stdout a representation of the query tree as either
  // compiled for (Detailed == true) or as a relational algebra expression.
  // This was originally written for an early prototype and needs to be 
  // rewritten for use in MySQL.
  // Warning: This is a RECURSIVE method!
  //**************************************************************************
  void PrintQuery_tree(bool Detailed, query_node *n);

  //**************************************************************************
  // BuildQueryString (public) 
  //**************************************************************************
  // This method returns the SQL statement for the query tree (cool beans!).
  //**************************************************************************
  char *BuildQueryString(query_node *QN);

  //**************************************************************************
  // HOptimization (public) 
  //**************************************************************************
  // This method performs heuristic optimization on the query tree. The 
  // operation is destructive.
  //**************************************************************************
  void HOptimization();

  //**************************************************************************
  // COptimization (public) 
  //**************************************************************************
  // This method performs cost-based optimization on the query tree. The 
  // operation is nondestructive.
  //**************************************************************************
  void COptimization();

  //**************************************************************************
  // SetDatabase (public) 
  //**************************************************************************
  // This method sets the default database name for the query.
  //**************************************************************************
  void SetDatabase(char *db);

  //**************************************************************************
  // GetDatabase (public) 
  //**************************************************************************
  // This method returns the current default database for the query tree.
  //**************************************************************************
  char *GetDatabase();

  //**************************************************************************
  // InsertNode (public) 
  //**************************************************************************
  // This method inserts a query node into the query tree starting at POS
  // with respect to the parent id (p) and the child path (c). 
  // Warning: This is a RECURSIVE method!
  //**************************************************************************
  void InsertNode(query_node *pos,
          query_node *QN, int p, char *c);

  //**************************************************************************
  // InsertRelation (public) 
  //**************************************************************************
  // This method is used to place a relation (r) in the query node (QN).
  //**************************************************************************
  bool InsertRelation(query_node *QN, Relation *r);

  //**************************************************************************
  // InsertAttribute (public) 
  //**************************************************************************
  // This method is used to place an attribute (c) in the query node (QN).
  //**************************************************************************
  void InsertAttribute(query_node *QN, Attribute::AttrStruct c);

  //**************************************************************************
  // SaveTree (public) 
  //**************************************************************************
  // This method saves the current query tree to a file (fn) in compiled form.
  //**************************************************************************
  //void SaveTree(char *fn);

  //**************************************************************************
  // Optimized (public) 
  //**************************************************************************
  // This method returns true if cost-based and heuristic optimization have
  // been applied to the query tree.
  //**************************************************************************
  bool Optimized(void);

  //**************************************************************************
  // FindSubQuery (public) 
  //**************************************************************************
  // This method is used to locate a sub query in the tree. It returns the 
  // node that is the root of the subquery.
  //**************************************************************************
  query_node *FindSubQuery(query_node *QN);

  //**************************************************************************
  // GetTables (public) 
  //**************************************************************************
  // This method returns a character string to return a list of tables in the
  // query tree from the node specified down.
  // Warning: This is a RECURSIVE method!
  //**************************************************************************
  char *GetTables(query_node *QN, char *Joins);

  //**************************************************************************
  // GetDistinct (public) 
  //**************************************************************************
  // This method returns the distinct select option setting.
  //**************************************************************************
  bool GetDistinct();

  //**************************************************************************
  // SetDistinct (public) 
  //**************************************************************************
  // This method sets the distinct select option to the bool passed. 
  //**************************************************************************
  void SetDistinct(bool Value);

    bool FindHiddenAttrs(query_node *N);
    static Attribute *GetAttribfromLeftTree(query_node *QN);
    static Attribute *GetAttribfromRightTree(query_node *QN);
    AggregateType Aggregate;            //what aggregate type we're supporting

private:
  char *Database;            //database name for query (default)
  bool HOpt;              //has query been optimized (rules)?
  bool COpt;              //has query been optimized (cost)?
  char *SetPath;            //the path to the query relations
  bool Distinct;            //distinct option for selects

  //**************************************************************************
  // WriteTreeNode (private) 
  //**************************************************************************
  // This method writes the current tree node to the file specified.
  //**************************************************************************
  //void WriteTreeNode(ofstream hFile, query_node *QN);

  //**************************************************************************
  // ReadTreeNode (private) 
  //**************************************************************************
  // This method reads the next tree node from the file specified.
  //**************************************************************************
  //void ReadTreeNode(ifstream hFile, query_node *QN);

  //**************************************************************************
  // PrintQuery_treeNode (private) 
  //**************************************************************************
  // This method writes to stdout the current tree node in relational
  // algebra format. This method was originally written for an early
  // prototype and should be rewritten for MySQL.
  //**************************************************************************
  void PrintQuery_treeNode(query_node *n);

  //**************************************************************************
  // PrintQuery_treeNodeDetail (private) 
  //**************************************************************************
  // This method writes to stdout the current tree node in a compiled 
  // detail format. This method was originally written for an early
  // prototype and should be rewritten for MySQL.
  //**************************************************************************
  void PrintQuery_treeNodeDetail(query_node *n);

  //**************************************************************************
  // NodeTypeString (private) 
  //**************************************************************************
  // This method returns an English statement for the node type (operation).
  //**************************************************************************
  char *NodeTypeString (query_node_type nt);

  //**************************************************************************
  // join_con_typeShortStr (private) 
  //**************************************************************************
  // This method returns an English statement for the join condition type.
  // This is an abbreviated form for use in relational algrebra statments.
  //**************************************************************************
  char *join_con_typeShortStr (join_con_type jc);

  //**************************************************************************
  // JoinConString (private) 
  //**************************************************************************
  // This method returns an English statement for the join condition type.
  //**************************************************************************
  char *JoinConString (join_con_type jc);

  //**************************************************************************
  // GetAttributes (private) 
  //**************************************************************************
  // This method returns a formatted string listing all of the attributes
  // referenced in the query.
  //**************************************************************************
  char *GetAttributes(query_node *QN);

  //**************************************************************************
  // GetExpressions (private) 
  //**************************************************************************
  // This method returns a formatted string listing all of the expressions
  // referenced in the query.
  //**************************************************************************
  //char *GetExpressions(query_node *QN, bool First = false);

  //**************************************************************************
  // GetJoins (private) 
  //**************************************************************************
  // This method returns a formatted string listing all of the joins
  // referenced in the query. Used in the FOR clause.
  //**************************************************************************
  char *GetJoins(query_node *QN);

  //**************************************************************************
  // JoinTypeString (private) 
  //**************************************************************************
  // This method returns an English statement for the join type.
  //**************************************************************************
  char *JoinTypeString(type_join jn);

  //**************************************************************************
  // JoinTypeShortStr (private) 
  //**************************************************************************
  // This method returns an English statement for the join type in an 
  // abbreviated form for use in relational algebra statements.
  //**************************************************************************
  char *JoinTypeShortStr(type_join jn);

  //**************************************************************************
  // GetTableFromExpression (private) 
  //**************************************************************************
  // This method returns the relation name for an expression.
  //**************************************************************************
  char *GetTableFromExpression(char *Exp);

  //**************************************************************************
  // PushProjections (private) 
  //**************************************************************************
  // This method looks for projections and pushes them down the tree to nodes
  // that contain the relations specified.
  // Warning: This is a RECURSIVE method!
  //**************************************************************************
  void PushProjections(query_node *QN, query_node *pNode);

  //**************************************************************************
  // FindProjection (private) 
  //**************************************************************************
  // This method looks for a node containing a projection and returns the node
  // pointer.
  // Warning: This is a RECURSIVE method!
  //**************************************************************************
  query_node *FindProjection(query_node *QN);

  //**************************************************************************
  // IsLeaf (private) 
  //**************************************************************************
  // This method returns TRUE if the node specified is a leaf (no children).
  //**************************************************************************
  bool IsLeaf(query_node *QN);

  //**************************************************************************
  // HasRelation (private) 
  //**************************************************************************
  // This method returns TRUE if the node contains the relation specified.
  //**************************************************************************
  bool HasRelation(query_node *QN, char *Table);

  //**************************************************************************
  // HasAttribute (private) 
  //**************************************************************************
  // This method returns TRUE if the node contains the attribute specified.
  //**************************************************************************
  bool HasAttribute(query_node *QN, Attribute::AttrStruct a);

  //**************************************************************************
  // DelAttribute (private) 
  //**************************************************************************
  // This method removes the attrbiute specified.
  //**************************************************************************
  void DelAttribute(query_node *QN, Attribute::AttrStruct a);

  //**************************************************************************
  // DelRelation (private) 
  //**************************************************************************
  // This method removes the relation specified.
  //**************************************************************************
  void DelRelation(query_node *QN, Relation *r);

  //**************************************************************************
  // HasExpression (private) 
  //**************************************************************************
  // This method returns TRUE if the node contains the expression specified.
  //**************************************************************************
  //bool HasExpression(query_node *QN, Expression::ExprStruct *a);

  //**************************************************************************
  // PushRestrictions (private) 
  //**************************************************************************
  // This method looks for restrictions and pushes them down the tree to nodes
  // that contain the relations specified.
  // Warning: This is a RECURSIVE method!
  //**************************************************************************
  void PushRestrictions(query_node *QN, query_node *pNode);

  //**************************************************************************
  // FindRestriction (private) 
  //**************************************************************************
  // This method looks for a node containing a restriction and returns the 
  // node pointer.
  //**************************************************************************
  query_node *FindRestriction(query_node *QN);

  //**************************************************************************
  // IsJoinCondition (private) 
  //**************************************************************************
  // This method returns TRUE if the expression is a join condition -- it has
  // a table for both the left and right parameters.
  //**************************************************************************
  bool IsJoinCondition(Expression::ExprStruct *e);

  //**************************************************************************
  // FindNaturalJoin (private) 
  //**************************************************************************
  // This method looks for a node containing a theta join and returns the 
  // node pointer.
  //**************************************************************************
  query_node *FindNaturalJoin(query_node *QN);

  //**************************************************************************
  // GetJoinCondition (private) 
  //**************************************************************************
  // This method returns the join expression from the node.
  //**************************************************************************
  //Expression::ExprStruct *GetJoinCondition(query_node *QN);

  //**************************************************************************
  // PushNaturalJoins (private) 
  //**************************************************************************
  // This method looks for theta joins and pushes them down the tree to the 
  // parent of two nodes that contain the relations specified.
  // Warning: This is a RECURSIVE method!
  //**************************************************************************
  void PushNaturalJoins(query_node *QN, query_node *pNode);

  //**************************************************************************
  // PruneTree (private) 
  //**************************************************************************
  // This method looks for nodes blank nodes that are a result of performing
  // heuristic optimization on the tree and deletes them.
  // Warning: This is a RECURSIVE method!
  //**************************************************************************
  void PruneTree(query_node *Prev, query_node *CurNode);

  //**************************************************************************
  // BalanceJoins (private) 
  //**************************************************************************
  // This method will balance the joins once cost-based factors are applied.
  // NOTE: This method is not completed yet!
  //**************************************************************************
  void BalanceJoins(query_node *QN);

  //**************************************************************************
  // SplitRestrictWithProject (private) 
  //**************************************************************************
  // This method looks for restrictions that have attributes (thus are both
  // projections and restrictions) and breaks them into two nodes.
  // Warning: This is a RECURSIVE method!
  //**************************************************************************
  void SplitRestrictWithProject(query_node *QN);

  //**************************************************************************
  // SplitRestrictWithJoin (private) 
  //**************************************************************************
  // This method looks for restrictions that have joins (thus are both
  // joins and restrictions) and breaks them into two nodes.
  // Warning: This is a RECURSIVE method!
  //**************************************************************************
  void SplitRestrictWithJoin(query_node *QN);
    void SplitProjectWithJoin(query_node *QN);

    bool FindTableinTree(query_node *QN, char *tbl);
    bool FindTableinExpr(Expr::Expr *e, char *tbl);
    bool FindAttrinExpr(Expr::Expr *e, char *tbl, char *value);

  //**************************************************************************
  // ApplyIndexes (private) 
  //**************************************************************************
  // This method will look for the best index for each node.
  // NOTE: This method is not completed yet!
  //**************************************************************************
  void ApplyIndexes(query_node *QN);
*/
};
