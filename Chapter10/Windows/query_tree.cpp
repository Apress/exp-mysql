/*
  Query_tree.cc

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
    query_tree.h
*/
#include "query_tree.h"

Query_tree::query_node::query_node()
{
  where_expr = NULL;
  join_expr = NULL;
  child = false;
  join_cond = Query_tree::jcUN;
  join_type = Query_tree::jnUNKNOWN;
  left = NULL;
  right = NULL;
  nodeid = -1;
  node_type = Query_tree::qntUndefined;
  sub_query = false;
  parent_nodeid = -1;
}

Query_tree::query_node::~query_node()
{
  if(left)
    delete left;
  if(right)
    delete right;
}

Query_tree::~Query_tree(void)
{
  if(root)
    delete root;
}

////******************************************************************************
//// Optimized (public)
////******************************************************************************
//// Purpose
//// This method returns true if cost-based and heuristic optimization have
//// been applied to the query tree.
////
//// Parameters
//// None
////
//// Returns
//// bool -- true = query tree has been optimized.
////
//// Error Handling
//// None
////
//// Implementation Notes
//// None
////******************************************************************************
//bool Query_tree::Optimized(void)
//{
//  ALV_IN("Optimized(void)");
//  ALV_OUT();
//  return HOpt && COpt;
//}
//
//
////******************************************************************************
//// SaveTree (public)
////******************************************************************************
//// Purpose
//// This method saves the current query tree to a file (fn) in compiled form.
////
//// Parameters
//// char *fn -- the full path and file name of the file to write to.
////
//// Returns
//// N/A
////
//// Error Handling
//// None
////
//// Implementation Notes
//// None
////******************************************************************************
///*
//void Query_tree::SaveTree(char *fn)
//{  
//  ALV_IN("SaveTree(char *fn)");
//  ofstream        hFile;
//  char            *s;
//
//  s = new char[255];
//  hFile.open(fn);
//  if(!hFile.bad())
//  {
//    //
//    //Write header
//    //
//    strcpy(s, "//\n//The file layout is as follows:\n//\n");
//    strcat(s, "//Database Name (occurs only once)\n");
//    hFile.write(s, strlen(s));
//    strcpy(s, "//1. SubQuery? 0 = NO, 1 = YES\n");
//    hFile.write(s, strlen(s));
//    strcpy(s, "//2. Parent Node Id (integer)\n");
//    strcat(s, "//3. Child path (L = left or R = right or U = undefined a.k.a. root)\n");
//    hFile.write(s, strlen(s));
//    strcpy(s, "//4. Node Id (integer)\n");
//    strcat(s, "//5. Node Type (0 = UNDEFINED, 1 = RESTRICT, 2 = PROJECT, 3 = JOIN, 5 = DISTINCT)\n");
//    hFile.write(s, strlen(s));
//    strcpy(s, "//6. Join Type (0=UNDEFINDED, 1=INNER, 2=LEFT OUTER, 3=RIGHT OUTER, 4=FULL OUTER, 5=CROSS PRODUCT, 6 = UNION, 7 = INTERSECT\n");
//    strcat(s, "//7. Join Condition (0=UNDEFINED, 1=NATURAL, 2=ON, 3=USING)\n");
//    hFile.write(s, strlen(s));
//    strcpy(s, "//8. #relations *** one per line\n");
//    strcat(s, "//    -Relation (string)\n");
//    hFile.write(s, strlen(s));
//    strcpy(s, "//   #columns (integer) *** One set of the following per # > 0\n");
//    strcat(s, "//    -Table (string)\n");
//    hFile.write(s, strlen(s));
//    strcpy(s, "//    -Column (string)\n");
//    strcat(s, "//   #Expressions (integer) *** One set of the following per # > 0\n");
//    hFile.write(s, strlen(s));
//    strcpy(s, "//    -Junction Type (0 = NONE, 1 = AND, 2 = OR)\n");
//    strcat(s, "//    -Operation (0='=', 1='<', 2='>', 3='<=', 4='>=', 5='IN', 6='LIKE', 7='<>')\n");
//    hFile.write(s, strlen(s));
//    strcpy(s, "//    -left operand table (string)\n");
//    strcat(s, "//    -left operand field (string)\n");
//    hFile.write(s, strlen(s));
//    strcpy(s, "//    -right operand table (string)\n");
//    strcat(s, "//    -right operand field/expression (string)\n");
//    hFile.write(s, strlen(s));
//    s = GetDatabase();
//    strcat(s, "\n");
//    hFile.write(s, strlen(s));
//    WriteTreeNode(hFile, Root);
//  }
//  hFile.close();
//  ALV_OUT();
//}
//*/
//
////******************************************************************************
//// ReadTree (public)
////******************************************************************************
//// Purpose
//// This method reads a compiled query from a file (passed) and builds the
//// internal tree for it.
////
//// Parameters
//// char *fn -- the full path and file name of the file to read.
////
//// Returns
//// N/A
////
//// Error Handling
//// None
////
//// Implementation Notes
//// None
////******************************************************************************
///*
//bool Query_tree::ReadTree(char *fn)
//{
//  ALV_IN("ReadTree(char *fn)");
//  ifstream        hFile;
//  bool            Success = true;
//  int             Parent;
//  int             i = 0;
//  char            s[255];
//  char            Child[24];
//  bool            SubQuery = false;
//  query_node *QN;
//
//  hFile.open(fn);
//  if(hFile.bad())
//  {
//    Success = false;
//  }
//  else
//  {
//    hFile.getline(s, 255, '\n');    //read node id
//    while(s[0] == '/')
//    {
//      //
//      //skip comment lines
//      //
//      hFile.getline(s, 255, '\n');    //read node id
//    }
//    SetDatabase(s);
//    while(!hFile.eof())
//    {
//      SubQuery = false;
//      hFile.getline(s, 255, '\n');
//      SubQuery = (atoi(s) > 0);
//      hFile.getline(s, 255, '\n');    //read node id
//      Parent = atoi(s);
//      hFile.getline(s, 255, '\n');    //read child (L/R)
//      strcpy(Child, s);
//      if(strlen(Child) > 0)
//      {
//        QN = new query_node;
//        QN->Attributes = new Attribute();
//        QN->SubQuery = SubQuery;
//        QN->Child = Child;
//        ReadTreeNode(hFile, QN);
//
//        //
//        //Now find out where it goes in the tree
//        //
//        if(Parent == -1)
//        {
//          Root = QN;
//          Root->ParentNodeId = -1;
//        }
//        else
//        {
//          InsertNode(Root, QN, Parent, Child);
//        }
//      }
//    }
//  }
//  ALV_OUT();
//  return Success;
//}
//*/
//
////******************************************************************************
//// WriteTreeNode (private) 
////******************************************************************************
//// Purpose
//// This method writes the current tree node to the file specified.
////
//// Parameters
//// ofstream hFile -- the file stream to write to.
//// query_node *QN -- the starting node to be operated on.
////
//// Returns
//// N/A
////
//// Error Handling
//// None
////
//// Implementation Notes
//// None
////******************************************************************************
///*
//void Query_tree::WriteTreeNode(ofstream hFile, query_node *QN)
//{
//  ALV_IN("WriteTreeNode(ofstream hFile, query_node *QN)");
//  char *s = new char[255];
//  int     i = 0;
//  int     j = 0;
//  Attribute::AttrStruct a;
//  Attribute   *Attrs = 0;
//  Expression::ExprStruct *e = 0;
//
//  if(QN->SubQuery)
//  {
//    hFile.write("1\n", strlen("1\n"));
//  }
//  else
//  {
//    hFile.write("0\n", strlen("0\n"));
//  }
//  itoa(QN->ParentNodeId, s, 10);
//  strcat(s, "\n");
//  hFile.write(s, strlen(s));
//
//  strcpy(s, QN->Child);
//  strcat(s, "\n");
//  hFile.write(s, strlen(s));
//
//  itoa(QN->NodeId, s, 10);
//  strcat(s, "\n");
//  hFile.write(s, strlen(s));
//
//  itoa((int)QN->NodeType, s, 10);
//  strcat(s, "\n");
//  hFile.write(s, strlen(s));
//
//  itoa((int)QN->JoinType, s, 10);
//  strcat(s, "\n");
//  hFile.write(s, strlen(s));
//
//  itoa((int)QN->JoinCondition, s, 10);
//  strcat(s, "\n");
//  hFile.write(s, strlen(s));
//
//  //Write relations
//  j = 0;
//  while(QN->Relations[j] != 0)
//  {
//    j++;
//  }
//  itoa(j, s, 10);
//  strcat(s, "\n");
//  hFile.write(s, strlen(s));
//  j = 0;
//  while(QN->Relations[j] != 0)
//  {
//    strcpy(s, QN->Relations[j]->GetTableName());
//    strcat(s, "\n");
//    hFile.write(s, strlen(s));
//    j++;
//  }
//
//  //Write attributes
//  Attrs = QN->Attributes;
//  j = Attrs->NumAttributes();
//  itoa(j, s, 10);
//  strcat(s, "\n");
//  hFile.write(s, strlen(s));
//  for(i = 0; i < j; i++)
//  {
//    a = Attrs->GetAttribute(i);
//    strcpy(s, a.Table);
//    strcat(s, "\n");
//    hFile.write(s, strlen(s));
//    strcpy(s, a.Value);
//    strcat(s, "\n");
//    hFile.write(s, strlen(s));
//  }
//
//  //Write expressions
//  if(QN->where_expr)
//  {
//    char *estring = QN->where_expr->ToString();
//         hFile.write(estring, strlen(estring));
//    delete estring;
//  }
//  
//  if(QN->Left != 0)
//  {
//    WriteTreeNode(hFile, QN->Left);
//  }
//  if(QN->Right != 0)
//  {
//    WriteTreeNode(hFile, QN->Right);
//  }
//  ALV_OUT();
//}
//*/
//
////******************************************************************************
//// PrintQuery_tree (public)
////******************************************************************************
//// Purpose
//// This method prints to stdout a representation of the query tree as either
//// compiled for (Detailed == true) or as a relational algebra expression.
//// This was originally written for an early prototype and needs to be 
//// rewritten for use in MySQL.
//// Warning: This is a RECURSIVE method!
////
//// Parameters
//// bool Detailed -- if true, print compiled version of the tree.
//// query_node *n -- the starting node to be operated on.
////
//// Returns
//// N/A
////
//// Error Handling
//// None
////
//// Implementation Notes
//// None
////******************************************************************************
//void Query_tree::PrintQuery_tree(bool Detailed, query_node *n)
//{
//  ALV_IN("PrintQuery_tree(bool Detailed, query_node *n)");
//  if(n != 0)
//  {
//    if(Detailed)
//    {
//      PrintQuery_treeNodeDetail(n);
//      PrintQuery_tree(Detailed, n->Left);
//      PrintQuery_tree(Detailed, n->Right);
//    }
//    else
//    {
//      if(n->NodeType == qntJoin)
//      {
//        if(n->Left != 0)
//        {
//          puts(" (");
//          PrintQuery_tree(Detailed, n->Left);
//          puts(") ");
//        }
//        PrintQuery_treeNode(n);
//        if(n->Left != 0)
//        {
//          puts(" (");
//          PrintQuery_tree(Detailed, n->Right);
//          puts(") ");
//        }
//      }
//      else
//      {
//        PrintQuery_treeNode(n);
//        if((n->Left != 0) || (n->Right != 0))
//        {
//          puts(" (");
//        }
//        PrintQuery_tree(Detailed, n->Left);
//        PrintQuery_tree(Detailed, n->Right);
//        if((n->Left != 0) || (n->Right != 0))
//        {
//          puts(") ");
//        }
//      }
//    }
//  }
//  ALV_OUT();
//}
//
////******************************************************************************
//// PrintQuery_treeNode (private)
////******************************************************************************
//// Purpose
//// This method writes to stdout the current tree node in relational
//// algebra format. This method was originally written for an early
//// prototype and should be rewritten for MySQL.
////
//// Parameters
//// query_node *n -- the starting node to be operated on.
////
//// Returns
//// N/A
////
//// Error Handling
//// None
////
//// Implementation Notes
//// None
////******************************************************************************
//void Query_tree::PrintQuery_treeNode(query_node *n)
//{
//  ALV_IN("PrintQuery_treeNode(query_node *n)");
//  char            Restrict;
//  char            Project;
//  char            Join;
//  Expression::ExprStruct *ex = 0;
//
//  Project = (char) 227;
//  Restrict = (char) 229;
//  Join = (char) 233;
//  if(n != 0)
//  {
//    switch(n->NodeType)
//    {
//    case qntSort:
//      {
//        puts("S ");
//        if(n->Attributes->NumAttributes() == 0)  // the trivial project
//        {
//          if(n->Relations[0] != 0)
//          {
//            puts(n->Relations[0]->GetTableName());
//          }
//        }
//        puts(n->Attributes->ToString(true));
//        break;
//      }
//    case qntProject:
//      {
//        printf("%d ", Project);
//        if(n->Attributes->NumAttributes() == 0)  // the trivial project
//        {
//          if(n->Relations[0] != 0)
//          {
//            puts(n->Relations[0]->GetTableName());
//          }
//        }
//        puts(n->Attributes->ToString(true));
//        break;
//      }
//    case qntRestrict:
//      {
//        printf("%d ", Restrict);
//        char *estr = n->where_expr->ToString();
//        puts(estr);
//        delete estr;
//        break;
//      }
//    case qntJoin:
//      {
//        if((n->JoinType != jnUNION) && 
//           (n->JoinType != jnINTERSECT))
//        {
//          printf("%d%s ", Join, JoinTypeShortStr(n->JoinType));
//          char *exp;
//          exp = n->join_expr->ToString();
//          printf("USING (%s)",exp);
//          delete exp;
//        }
//        else 
//        if((n->JoinType == jnUNION) ||
//            (n->JoinType == jnINTERSECT))
//        {
//          //
//          //it's a union or intersect!
//          //
//          if(n->Relations[0] != 0)
//          {
//            puts(n->Relations[0]->GetTableName());
//          }
//          printf(" %s ", JoinTypeShortStr(n->JoinType));
//          if(n->Relations[1] != 0)
//          {
//            puts(n->Relations[1]->GetTableName());
//          }
//        }
//        else if(n->JoinType != jnCROSSPRODUCT)
//        {
//          puts("ON ");
//          char *exp;
//          exp = n->join_expr->ToString();
//          puts(exp);
//          delete exp;
//        }
//        else // it's a cross product.
//        {
//          printf("%s X ", n->Relations[0]->GetTableName());
//          puts(n->Relations[1]->GetTableName());
//        }
//        break;
//      }
//    }
//  }
//  ALV_OUT();
//}
//
////******************************************************************************
//// PrintQuery_treeNodeDetail (private)
////******************************************************************************
//// Purpose
//// This method writes to stdout the current tree node in a compiled 
//// detail format. This method was originally written for an early
//// prototype and should be rewritten for MySQL.
////
//// Parameters
//// query_node *n -- the starting node to be operated on.
////
//// Returns
//// N/A
////
//// Error Handling
//// None
////
//// Implementation Notes
//// None
////******************************************************************************
//void Query_tree::PrintQuery_treeNodeDetail(query_node *n)
//{
//  ALV_IN("PrintQuery_treeNodeDetail(query_node *n)");
//  int     i;
//
//  if(n != 0)
//  {
//    if(n->SubQuery)
//    {
//      puts("SUBQUERY\n");
//    }
//    printf("NodeId: %d\n", n->NodeId);
//    printf("Parent NodeId: %d\n", n->ParentNodeId);
//    printf("NodeType: %s\n", NodeTypeString(n->NodeType));
//    //printf("Cost: %f\n", n->Cost);
//    //printf("Size: %d\n", n->Size);
//    if(n->NodeType == qntJoin)
//    {
//      printf("Join Type: %s\n", JoinTypeString(n->JoinType));
//      printf("Join Condition %s\n", JoinConString(n->JoinCondition));
//    }
//    printf("Relations: ");
//    if(n->Relations[0] == 0)
//    {
//      printf("<none>");
//    }
//    i = 0;
//    while(i < MAXNODETABLES)
//    {
//      if(n->Relations[i] != 0)
//      {
//        printf(n->Relations[i]->GetTableName());
//        if((i+1) < MAXNODETABLES)
//        {
//          if(n->Relations[i+1] != 0)
//          {
//            printf(", ");
//          }
//        }
//      }
//      i++;
//    }
//    printf("\nAttributes: ");
//    if(n->Attributes->NumAttributes() == 0)
//    {
//      printf("<none>");
//    }
//    else
//    {
//      printf(n->Attributes->ToString());
//    }
//    printf("\nWHERE Clause Expressions: ");
//    
//    if(n->where_expr == NULL)
//    {
//      printf("<none>");
//    }
//    else
//    {
//      char *exp = n->where_expr->ToString();
//      printf("%s", exp);
//      delete exp;
//    }
//    
//    printf("\nJOIN Clause Expressions: ");
//    if(n->join_expr == NULL)
//    {
//      printf("<none>");
//    }
//    else
//    {
//      char *exp = n->join_expr->ToString();
//      printf("%s", exp);
//      delete exp;
//    }
//
//        if(n->Left != 0)
//    {
//      printf("\nLeft Child Id: %d\n", n->Left->NodeId);
//    }
//    if(n->Right != 0)
//    {
//      printf("\nRight Child Id: %d\n", n->Right->NodeId);
//    }
//    printf("\n\n");
//  }
//  ALV_OUT();
//}
//
////******************************************************************************
//// InsertNode (public) 
////******************************************************************************
//// Purpose
//// This method inserts a query node into the query tree starting at POS
//// with respect to the parent id (p) and the child path (c). 
//// Warning: This is a RECURSIVE method!
////
//// Parameters
//// query_node *pos -- the starting node to be operated on.
//// query_node *QN -- the node to be inserted.
//// int p -- the parent's ndoe id.
//// char *c -- the child's alignment (L or R)
////
//// Returns
//// N/A
////
//// Error Handling
//// None
////
//// Implementation Notes
//// None
////******************************************************************************
//void Query_tree::InsertNode(query_node *pos,
//               query_node *QN, int p, char *c)
//{
//  ALV_IN("InsertNode(query_node *pos, query_node *QN, int p, char *c)");
//  if(pos != 0)
//  {
//    if(pos->NodeId == p)
//    {
//      if(strcmp(c, "L") == 0)
//      {
//        pos->Left = QN;
//      }
//      else
//      {
//        pos->Right = QN;
//      }
//      QN->ParentNodeId = p;
//    }
//    else
//    {
//      InsertNode(pos->Left, QN, p, c);  //look left
//      InsertNode(pos->Right, QN, p, c);  //look right
//    }
//  }
//  ALV_OUT();
//}
//
////******************************************************************************
//// NodeTypeString (private) 
////******************************************************************************
//// Purpose
//// This method returns an English statement for the node type (operation).
////
//// Parameters
//// query_node_type nt -- the type to be converted to a string
////
//// Returns
//// char * -- the converted string
////
//// Error Handling
//// None
////
//// Implementation Notes
//// None
////******************************************************************************
//char *Query_tree::NodeTypeString (query_node_type nt)
//{
//  ALV_IN("NodeTypeString (query_node_type nt)");
//  char *s;
//
//  s = (char *)malloc(12);
//
//  strcpy (s, "");
//  switch(nt)
//  {
//  case qntUndefined :
//    strcpy(s, "UNDEFINED");
//    break;
//  case qntRestrict :
//    strcpy(s, "RESTRICT");
//    break;
//  case qntProject :
//    strcpy(s, "PROJECT");
//    break;
//  case qntJoin:
//    strcpy(s, "JOIN");
//    break;
//  case qntSort:
//    strcpy(s, "SORT");
//    break;
//  case qntDistinct:
//    strcpy(s, "DISTINCT");
//    break;
//  }
//  ALV_OUT();
//  return s;
//}
//
////******************************************************************************
//// JoinTypeShortStr (private) 
////******************************************************************************
//// Purpose
//// This method returns an English statement for the join type in an 
//// abbreviated form for use in relational algebra statements.
////
//// Parameters
//// type_join jn -- the type to be converted to a string
////
//// Returns
//// char * -- the converted string
////
//// Error Handling
//// None
////
//// Implementation Notes
//// None
////******************************************************************************
//char *Query_tree::JoinTypeShortStr (type_join jn)
//{
//  ALV_IN("JoinTypeShortStr (type_join jn)");
//  char *s;
//
//  s = (char *)malloc(15);
//
//  strcpy (s, "");
//  switch(jn)
//  {
//  case jnUNKNOWN :
//    strcpy(s, "UN");
//    break;
//  case jnINNER:
//    strcpy(s, "IN");
//    break;
//  case jnLEFTOUTER :
//    strcpy(s, "LO");
//    break;
//  case jnRIGHTOUTER :
//    strcpy(s, "RO");
//    break;
//  case jnFULLOUTER :
//    strcpy(s, "FO");
//    break;
//  case jnCROSSPRODUCT:
//    strcpy(s, "CP");
//    break;
//  case jnUNION:
//    strcpy(s, "[]");
//    break;
//  case jnINTERSECT:
//    strcpy(s, "][");
//    break;
//  }
//  ALV_OUT();
//  return s;
//}
//
////******************************************************************************
//// join_con_typeShortStr (private) 
////******************************************************************************
//// Purpose
//// This method returns an English statement for the join condition type.
//// This is an abbreviated form for use in relational algrebra statments.
////
//// Parameters
//// join_con_type jc -- the type to be converted to a string
////
//// Returns
//// char * -- the converted string
////
//// Error Handling
//// None
////
//// Implementation Notes
//// None
////******************************************************************************
//char *Query_tree::join_con_typeShortStr (join_con_type jc)
//{
//  ALV_IN("join_con_typeShortStr (join_con_type jc)");
//  char *s;
//
//  s = (char *)malloc(15);
//
//  strcpy (s, "");
//  switch(jc)
//  {
//  case jcUN :
//    strcpy(s, "UN");
//    break;
//  case jcNA:
//    strcpy(s, "NA");
//    break;
//  case jcON:
//    strcpy(s, "ON");
//    break;
//  case jcUS :
//    strcpy(s, "US");
//    break;
//  }
//  ALV_OUT();
//  return s;
//}
//
////******************************************************************************
//// JoinTypeString (private) 
////******************************************************************************
//// Purpose
//// This method returns an English statement for the join type.
////
//// Parameters
//// type_join jn -- the type to be converted to a string
////
//// Returns
//// char * -- the converted string
////
//// Error Handling
//// None
////
//// Implementation Notes
//// None
////******************************************************************************
//char *Query_tree::JoinTypeString (type_join jn)
//{
//  ALV_IN("JoinTypeString (type_join jn)");
//  char *s;
//
//  s = (char *)malloc(15);
//
//  strcpy (s, "");
//  switch(jn)
//  {
//  case jnUNKNOWN:
//    strcpy(s, "UNDEFINED");
//    break;
//  case jnINNER:
//    strcpy(s, "INNER JOIN");
//    break;
//  case jnLEFTOUTER :
//    strcpy(s, "LEFT OUTER");
//    break;
//  case jnRIGHTOUTER :
//    strcpy(s, "RIGHT OUTER");
//    break;
//  case jnFULLOUTER :
//    strcpy(s, "FULL OUTER");
//    break;
//  case jnCROSSPRODUCT:
//    strcpy(s, "CROSS PRODUCT");
//    break;
//  case jnUNION:
//    strcpy(s, "UNION");
//    break;
//  case jnINTERSECT:
//    strcpy(s, "INTERSECT");
//    break;
//  }
//  ALV_OUT();
//  return s;
//}
//
////******************************************************************************
//// JoinConString (private) 
////******************************************************************************
//// Purpose
//// This method returns an English statement for the join condition type.
////
//// Parameters
//// join_con_type jc -- the type to be converted to a string
////
//// Returns
//// char * -- the converted string
////
//// Error Handling
//// None
////
//// Implementation Notes
//// None
////******************************************************************************
//char *Query_tree::JoinConString (join_con_type jc)
//{
//  ALV_IN("JoinConString (join_con_type jc)");
//  char *s;
//
//  s = (char *)malloc(15);
//
//  strcpy (s, "");
//  switch(jc)
//  {
//  case jcUN:
//    strcpy(s, "UNDEFINED");
//    break;
//  case jcNA:
//    strcpy(s, "NATURAL");
//    break;
//  case jcON:
//    strcpy(s, "ON");
//    break;
//  case jcUS :
//    strcpy(s, "USING");
//    break;
//  }
//  ALV_OUT();
//  return s;
//}
//
////******************************************************************************
//// FindSubQuery (public) 
////******************************************************************************
//// Purpose
//// This method is used to locate a sub query in the tree. It returns the 
//// node that is the root of the subquery.
////
//// Parameters
//// query_node *QN -- the node to be operated on.
////
//// Returns
//// query_node * -- the node containing the subquery. Note: returns NULL if no
////                  subquery is found.
////
//// Error Handling
//// None
////
//// Implementation Notes
//// None
////******************************************************************************
//Query_tree::query_node *Query_tree::FindSubQuery(query_node *QN)
//{
//  ALV_IN("FindSubQuery(query_node *QN)");
//  query_node *N = 0;
//
//  if(QN != 0)
//  {
//    if(QN->SubQuery)
//    {
//      N = QN;
//    }
//    else
//    {
//      N = FindSubQuery(QN->Left);
//      if(N == 0)
//      {
//        N = FindSubQuery(QN->Right);
//      }
//    }
//  }
//  ALV_OUT();
//  return N;
//}
//
////******************************************************************************
//// BuildQueryString (public) 
////******************************************************************************
//// Purpose
//// This method returns the SQL statement for the query tree (cool beans!).
////
//// Parameters
//// query_node *QN -- the node to be operated on.
////
//// Returns
//// char * -- the SQL statement for this query.
////
//// Error Handling
//// None
////
//// Implementation Notes
//// None
////******************************************************************************
//char *Query_tree::BuildQueryString(query_node *QN)
//{
//  ALV_IN("BuildQueryString(query_node *QN)");
//  char        *SelString;
//  char        *Sub;
//  char        *Joins;
//  query_node   *N = 0;
//
//  SelString = new char[1024];
//  Sub = new char[255];
//  Joins = new char[255];
//  strcpy(Sub, "");
//
//  //
//  //find all projections and build list of columns
//  //
//  strcpy(SelString, "SELECT ");
//  if(Distinct)
//  {
//    strcat(SelString, "DISTINCT ");
//  }
//  if(strcmp(GetAttributes(QN), "") == 0)
//  {
//    strcat(SelString, "*");
//  }
//  else
//  {
//    strcat(SelString, GetAttributes(QN));
//  }
//
//  //
//  //find all tables (relations)
//  //
//  strcat(SelString, "\nFROM ");
//  N = FindSubQuery(QN->Left);
//  if(N != 0)
//  {
//    Sub = BuildQueryString(N);
//    if(strlen(Sub) > 0)
//    {
//      strcat(SelString, "(");
//      strcat(SelString, Sub);
//      strcat(SelString, ")");
//    }
//  }
//  strcpy(Joins, GetJoins(QN));
//  strcat(SelString, Joins);
//  N = FindSubQuery(QN->Right);
//  if(N != 0)
//  {
//    Sub = BuildQueryString(N);
//    if(strlen(Sub) > 0)
//    {
//      strcat(SelString, "(");
//      strcat(SelString, Sub);
//      strcat(SelString, ")");
//    }
//  }
//  strcat(SelString, GetTables(QN, Joins));
//  //
//  //find all Expressions on projections
//  //
//  if(QN->where_expr != NULL)
//  {
//    char *wstr = QN->where_expr->ToString();
//    strcat(SelString, "\nWHERE ");
//    strcat(SelString, wstr);
//    delete wstr;
//  }
//  ALV_OUT();
//  return SelString;
//}
//
////******************************************************************************
//// GetAttributes (private) 
////******************************************************************************
//// Purpose
//// This method returns a formatted string listing all of the attributes
//// referenced in the query.
////
//// Parameters
//// query_node *QN -- the node to be operated on.
////
//// Returns
//// char * -- the attributes for the query node.
////
//// Error Handling
//// None
////
//// Implementation Notes
//// None
////******************************************************************************
//char *Query_tree::GetAttributes(query_node *QN)
//{
//  ALV_IN("GetAttributes(query_node *QN)");
//  Attribute   *c;
//
//  c = QN->Attributes;
//  ALV_OUT();
//  return c->ToString(false);
//}
//
////******************************************************************************
//// GetTables (public) 
////******************************************************************************
//// Purpose
//// This method returns a character string to return a list of tables in the
//// query tree from the node specified down.
//// Warning: This is a RECURSIVE method!
////
//// Parameters
//// query_node *QN -- the node to be operated on.
//// char *Joins -- the current tables in the join conditions
////
//// Returns
//// char * -- the tables for the query node/tree.
////
//// Error Handling
//// None
////
//// Implementation Notes
//// None
////******************************************************************************
//char *Query_tree::GetTables(query_node *QN, char *Joins)
//{
//  ALV_IN("GetTables(query_node *QN, char *Joins)");
//  char        *Rels;
//  char        *TblL;
//  char        *TblR;
//  int         i = 0;
//  char        *Tbls;
//
//  Rels = new char[255];
//  TblL = new char[255];
//  TblR = new char[255];
//  Tbls = new char[255];
//  strcpy(Tbls, "");
//  strcpy(TblL, "");
//  strcpy(TblR, "");
//  strcpy(Rels, "");
//  if(QN != 0)
//  {
//    //
//    //Only get the tables from the nodes
//    //that are not participating in joins.
//    //
//    if(QN->NodeType != qntJoin)
//    {
//      //
//      //Get Relations
//      //
//      if(QN->Relations[0] != 0)
//      {
//        if(strstr(Joins, QN->Relations[0]->GetTableName()) == 0)
//        {
//          strcat(Rels, QN->Relations[0]->GetTableName());
//        }
//      }
//      else if(QN->Left != 0)
//      {
//        strcat(Rels, GetTables(QN->Left, Joins));
//      }
//    }
//    else
//    {
//      if(QN->Relations[0] != 0)
//      {
//        if(strstr(Joins, QN->Relations[0]->GetTableName()) == 0)
//        {
//          strcpy(TblL, QN->Relations[0]->GetTableName());
//        }
//      }
//      else if(QN->Left != 0)
//      {
//        if(!QN->Left->SubQuery)
//        {
//          strcat(TblL, GetTables(QN->Left, Joins));
//        }
//      }
//      if(QN->Relations[1] != 0)
//      {
//        if(strstr(Joins, QN->Relations[1]->GetTableName()) == 0)
//        {
//          strcpy(TblR, QN->Relations[1]->GetTableName());
//        }
//      }
//      else if(QN->Right != 0)
//      {
//        if(!QN->Right->SubQuery)
//        {
//          strcat(TblR, GetTables(QN->Right, Joins));
//        }
//      }
//    }
//    if((strstr(TblL, Rels) != 0) &&
//       (strstr(TblR, Rels) != 0))
//    {
//      if((strlen(TblL) > 0) && (strlen(Rels) > 0))
//      {
//        strcat(Rels, ", ");
//      }
//      strcat(Rels, TblL);
//      if((strlen(TblR) > 0) && (strlen(Rels) > 0))
//      {
//        strcat(Rels, ", ");
//      }
//      strcat(Rels, TblR);
//    }
//  }
//  delete TblL;
//  delete TblR;
//  ALV_OUT();
//  return Rels;
//}
//
////******************************************************************************
//// GetExpressions (private) 
////******************************************************************************
//// Purpose
//// This method returns a formatted string listing all of the expressions
//// referenced in the query.
////
//// Parameters
//// query_node *QN -- the node to be operated on.
//// bool First -- if true, this is the first call (don't need a connector
////               operation prepended)
////
//// Returns
//// char * -- the expressions for the query node/tree.
////
//// Error Handling
//// None
////
//// Implementation Notes
//// None
////******************************************************************************
///*
//char *Query_tree::GetExpressions(query_node *QN, bool First)
//{
//  ALV_IN("GetExpressions(query_node *QN, bool First)");
//  char        *Exp;
//
//  Exp = new char[255];
//  strcpy(Exp, "");
//  if(QN != 0)
//  {
//    if(QN->NodeType == qntRestrict)
//    {
//      //strcpy(Exp, QN->Expressions->ToString(""));
//      First = false;
//    }
//    if(QN->Left != 0)
//    {
//      if(!QN->Left->SubQuery)
//      {
//        if(strlen(GetExpressions(QN->Left)) > 0)
//        {
//          if(!First)
//          {
//            strcat(Exp, " AND ");
//          }
//          strcat(Exp, GetExpressions(QN->Left, First));
//          First = false;
//        }
//      }
//    }
//    if(QN->Right != 0)
//    {
//      if(!QN->Right->SubQuery)
//      {
//        if(strlen(GetExpressions(QN->Right)) > 0)
//        {
//          if(!First)
//          {
//            strcat(Exp, " AND ");
//          }
//          strcat(Exp, GetExpressions(QN->Right, First));
//          First = false;
//        }
//      }
//    }
//  }
//  ALV_OUT();
//  return Exp;
//}
//*/
//
////******************************************************************************
//// GetJoins (private) 
////******************************************************************************
//// Purpose
//// This method returns a formatted string listing all of the joins
//// referenced in the query. Used in the FOR clause.
////
//// Parameters
//// query_node *QN -- the node to be operated on.
////
//// Returns
//// char * -- the joins for the query node/tree.
////
//// Error Handling
//// None
////
//// Implementation Notes
//// None
////******************************************************************************
//char *Query_tree::GetJoins(query_node *QN)
//{
//  ALV_IN("GetJoins(query_node *QN)");
//  char        *Join;
//
//  Join = new char[255];
//  strcpy(Join, "");
//  if(QN != 0)
//  {
//    //
//    //If this node is a join type, it means
//    //the relations below are joined together
//    //based on the expression in this node by
//    //the join type specified.
//    //
//    if(QN->NodeType == qntJoin)
//    {
//      if(QN->Left != 0)
//      {
//        if(!QN->Left->SubQuery)
//        {
//          if(QN->Left->NodeType != qntJoin)
//          {
//            //
//            //get relation if not a subjoin
//            //
//            if(QN->Left->Relations[0] != 0)
//            {
//              strcat(Join, QN->Left->Relations[0]->GetTableName());
//            }
//          }
//        }
//      }
//      else
//      {
//        strcat(Join, QN->Relations[0]->GetTableName());
//      }
//      strcat(Join, " ");
//      switch(QN->JoinType)
//      {
//      case jnINNER:
//        {
//          strcat(Join, "INNER JOIN");
//          break;
//        }
//      case jnLEFTOUTER:
//        {
//          strcat(Join, "LEFT OUTER JOIN");
//          break;
//        }
//      case jnRIGHTOUTER:
//        {
//          strcat(Join, "RIGHT OUTER JOIN");
//          break;
//        }
//      case jnFULLOUTER:
//        {
//          strcat(Join, "FULL OUTER JOIN");
//          break;
//        }
//      case jnUNION:
//        {
//          strcat(Join, "UNION");
//          break;
//        }
//      case jnINTERSECT:
//        {
//          strcat(Join, "INTERSECT");
//          break;
//        }
//      case jnCROSSPRODUCT:
//        {
//          strcat(Join, "CROSS");
//          break;
//        }
//      default:
//        {
//          strcat(Join, "JOIN");
//          break;
//        }
//      }
//      strcat(Join, " ");
//      if(QN->Right != 0)
//      {
//        if(!QN->Right->SubQuery)
//        {
//          if(QN->Right->NodeType != qntJoin)
//          {
//            //
//            //get relation if not a subjoin
//            //
//            if(QN->Right->Relations[0] != 0)
//            {
//              strcat(Join, QN->Right->Relations[0]->GetTableName());
//            }
//          }
//        }
//      }
//      else
//      {
//        strcat(Join, QN->Relations[1]->GetTableName());
//      }
//      if(QN->JoinCondition == jcON)
//      {
//        strcat(Join, " ON ");
//        
//        Expr::Expr *jexp = QN->join_expr;
//        if(jexp)
//        {
//          char *jstr = jexp->ToString();
//          strcat(Join, jstr);
//          delete jstr;
//        }
//        else
//          strcat(Join, "TRUE");  
//      }
//    }
//    if(QN->Left != 0)
//    {
//      if(!QN->Left->SubQuery)
//      {
//        strcat(Join, GetJoins(QN->Left));
//      }
//    }
//    if(QN->Right != 0)
//    {
//      if(!QN->Right->SubQuery)
//      {
//        strcat(Join, GetJoins(QN->Right));
//      }
//    }
//  }
//  ALV_OUT();
//  return Join;
//}
//
////******************************************************************************
//// GetTableFromExpression (private) 
////******************************************************************************
//// Purpose
//// This method returns the relation name for an expression.
////
//// Parameters
//// char *Exp -- the expression string to be operated on.
////
//// Returns
//// char * -- the relation(s) for the expression.
////
//// Error Handling
//// None
////
//// Implementation Notes
//// None
////******************************************************************************
//char *Query_tree::GetTableFromExpression(char *Exp)
//{
//  ALV_IN("GetTableFromExpression(char *Exp)");
//  char    *s;
//  char    *c;
//  int     i;
//
//  c = new char[25];
//  strcpy(c, "");
//  c = strstr(Exp, ".");
//  s = new char[25];
//  strcpy(s, "");
//  if(c != 0)
//  {
//    i = c - Exp + 1;
//    if(i > 0)
//    {
//      strncat(s, Exp, i - 1);
//    }
//  }
//  ALV_OUT();
//  return s;
//}
//
//
//bool Query_tree::FindHiddenAttrs(query_node *N)
//{
//    bool Found = false;
//
//    if (N!=0)
//    {
//        Found = FindHiddenAttrs(N->Left);
//        if (!Found)
//        {
//            Found = FindHiddenAttrs(N->Right);
//        }
//        if (!Found)
//        {
//            for (int i = 0; i < N->Attributes->NumAttributes();i++)
//            {
//                if (N->Attributes->GetAttribute(i).Hide)
//                {
//                    Found = true;
//                }
//            }
//        }
//    }
//    return Found;
//}
//
////******************************************************************************
//// HOptimization (public) 
////******************************************************************************
//// Purpose
//// This method performs heuristic optimization on the query tree. The 
//// operation is destructive.
////
//// Parameters
//// None
////
//// Returns
//// N/A
////
//// Error Handling
//// None
////
//// Implementation Notes
//// None
////******************************************************************************
//void Query_tree::HOptimization()
//{
//  ALV_IN("HOptimization()");
//  query_node       *pNode;
//  query_node       *nNode;
//
//  HOpt = true;
//  //
//  //First, we have to correct the situation where restrict and
//  //project are grouped together in the same node.
//  //
//  SplitRestrictWithJoin(Root);
///*
//    printf("\nAfter SplitRestrictWithJoin:\n\n");
//    PrintQuery_tree(true, Root);
//    if (FindHiddenAttrs(Root))
//    {
//        printf("Hidden attributes exist!\n");
//    }
//*/
//    SplitProjectWithJoin(Root);
///*
//    printf("\nAfter SplitProjectWithJoin:\n\n");
//    PrintQuery_tree(true, Root);
//    if (FindHiddenAttrs(Root))
//    {
//        printf("Hidden attributes exist!\n");
//    }
//*/
//    SplitRestrictWithProject(Root);
///*
//    printf("\nAfter SplitRestrictWithProject:\n\n");
//    PrintQuery_tree(true, Root);
//    if (FindHiddenAttrs(Root))
//    {
//        printf("Hidden attributes exist!\n");
//    }
//*/
//  //
//  //Find a node with restrictions and push down the tree using 
//  //a recursive call. continue until you get the same node twice.
//  //This means that the node cannot be pushed down any further.
//  //
//  pNode = FindRestriction(Root);
//  while(pNode != 0)
//  {
//    PushRestrictions(Root, pNode);
//    nNode = FindRestriction(Root);
//    if(nNode != 0)
//    {
//      if(nNode->NodeId == pNode->NodeId)
//      {
//        pNode = 0;
//      }
//      else if(IsLeaf(nNode))
//      {
//        pNode = 0;
//      }
//      else
//      {
//        pNode = nNode;
//      }
//    }
//  }
///*
//    printf("\nAfter PushRestrictions:\n\n");
//    PrintQuery_tree(true, Root);
//    if (FindHiddenAttrs(Root))
//    {
//        printf("Hidden attributes exist!\n");
//    }
//*/
//    //
//  //Find a node with projections and push down the tree using 
//  //a recursive call. Continue until you get the same node twice.
//  //This means that the node cannot be pushed down any further.
//  //
//  pNode = FindProjection(Root);
//  while(pNode != 0)
//  {
//    PushProjections(Root, pNode);
//    nNode = FindProjection(Root);
//    if(nNode != 0)
//    {
//      if(nNode->NodeId == pNode->NodeId)
//      {
//        pNode = 0;
//      }
//      else if(IsLeaf(nNode))
//      {
//        pNode = 0;
//      }
//      else
//      {
//        pNode = nNode;
//      }
//    }
//  }
///*
//    printf("\nAfter PushProjections:\n\n");
//    PrintQuery_tree(true, Root);
//    if (FindHiddenAttrs(Root))
//    {
//        printf("Hidden attributes exist!\n");
//    }
//*/
//    //
//  //Remove all cross products
//  //
//  pNode = FindNaturalJoin(Root);
//  while(pNode != 0)
//  {
//    PushNaturalJoins(Root, pNode);
//    nNode = FindNaturalJoin(Root);
//    if(nNode != 0)
//    {
//      if(nNode->NodeId == pNode->NodeId)
//      {
//        pNode = 0;
//      }
//      else if(IsLeaf(nNode))
//      {
//        pNode = 0;
//      }
//      else
//      {
//        pNode = nNode;
//      }
//    }
//    else
//    {
//      pNode = nNode;
//    }
//  }
//
//  //
//  //Prune the tree of "blank" nodes
//  //Blank Nodes are:
//  // 1) projections without attributes that have at least 1 child
//  // 2) restrictions without expressions
//  // BUT...Can't delete a node that has TWO children!
//  //
//  PruneTree(0, Root);
//
//    //
//  //Lastly, check to see if this has the DISTINCT option.
//  //If so, create a new node that is a DISTINCT operation.
//  //
//  if(Distinct && (Root->NodeType != qntDistinct))
//  {
//    int i;
//
//    pNode = new query_node;
//    pNode->Cost = 0;
//    pNode->Size = 0;
//    pNode->SubQuery = 0;
//    pNode->Attributes = 0;
//    pNode->JoinCondition = (join_con_type) 0;
//    pNode->JoinType = (type_join) 0;
//    pNode->Left = Root;
//    pNode->Right = 0;
//    for(i = 0; i < MAXNODETABLES; i++)
//    {
//      pNode->Relations[i] = 0;
//      //      pNode->SelIndex[i] = 0;
//    }
//    pNode->NodeId = 90125;
//    pNode->Child = "U";
//    Root->ParentNodeId = 90125;
//    strcpy(Root->Child, "L");
//    pNode->ParentNodeId = -1;
//    pNode->NodeType = qntDistinct;
//    pNode->Attributes = new Attribute();
//    Root = pNode;
//  }
//  ALV_OUT();
//}
//
////******************************************************************************
//// PruneTree (private) 
////******************************************************************************
//// Purpose
//// This method looks for nodes blank nodes that are a result of performing
//// heuristic optimization on the tree and deletes them.
//// Warning: This is a RECURSIVE method!
////
//// Parameters
//// query_node *Prev -- the previous node (parent)
//// query_node *CurNode -- the current node pointer (used to delete).
////
//// Returns
//// N/A
////
//// Error Handling
//// None
////
//// Implementation Notes
//// None
////******************************************************************************
//void Query_tree::PruneTree(query_node *Prev, query_node *CurNode)
//{
//  ALV_IN("PruneTree(query_node *Prev, query_node *CurNode)");
//  if(CurNode != 0)
//  {
//    //
//    //Blank Nodes are:
//    // 1) projections without attributes that have at least 1 child
//    // 2) restrictions without expressions
//    // BUT...Can't delete a node that has TWO children!
//    //
//    if((((CurNode->NodeType == qntProject) && 
//       (CurNode->Attributes->NumAttributes() == 0)) ||
//      ((CurNode->NodeType == qntRestrict) && 
//       (CurNode->where_expr == NULL))) &&
//       ((CurNode->Left == 0) || (CurNode->Right == 0)))
//    {
//      //
//      //if Prev == 0, we're at the root.
//      //
//      if(Prev == 0)
//      {
//        if(CurNode->Left == 0)
//        {
//          CurNode->Right->ParentNodeId = -1;
//          Root = CurNode->Right;
//        }
//        else
//        {
//          CurNode->Left->ParentNodeId = -1;
//          Root = CurNode->Left;
//        }
//        delete CurNode;
//        CurNode = Root;
//      }
//      else
//      {
//        if(Prev->Left == CurNode)
//        {
//          if(CurNode->Left == 0)
//          {
//            Prev->Left = CurNode->Right;
//            CurNode->Right->ParentNodeId = Prev->NodeId;
//          }
//          else
//          {
//            Prev->Left = CurNode->Left;
//            CurNode->Left->ParentNodeId = Prev->NodeId;
//          }
//          delete CurNode;
//          CurNode = Prev->Left;
//        }
//        else
//        {
//          if(CurNode->Left == 0)
//          {
//            Prev->Right = CurNode->Right;
//            CurNode->Right->ParentNodeId = Prev->NodeId;
//          }
//          else
//          {
//            Prev->Right = CurNode->Left;
//            CurNode->Left->ParentNodeId = Prev->NodeId;
//          }
//          delete CurNode;
//          CurNode = Prev->Right;
//        }
//      }
//      PruneTree(Prev, CurNode);
//    }
//    else
//    {
//      PruneTree(CurNode, CurNode->Left);
//      PruneTree(CurNode, CurNode->Right);
//    }
//  }
//  ALV_OUT();
//}
//
//
////******************************************************************************
//// FindProjection (private) 
////******************************************************************************
//// Purpose
//// This method looks for a node containing a projection and returns the node
//// pointer.
//// Warning: This is a RECURSIVE method!
////
//// Parameters
//// query_node *QN -- the node to operate on
////
//// Returns
//// query_node * -- the node located or NULL for not found
////
//// Error Handling
//// None
////
//// Implementation Notes
//// This finds the first projection and is biased to the left tree.
////******************************************************************************
//Query_tree::query_node *Query_tree::FindProjection(query_node *QN)
//{
//  ALV_IN("FindProjection(query_node *QN)");
//  query_node   *N;
//
//  N = 0;
//  //
//  //Look for the first node that is a projection that 
//  //has attributes and return it.
//  //
//  if(QN != 0)
//  {
//    if((QN->NodeType == qntProject) &&
//       (QN->Attributes != 0))
//    {
//      N = QN;
//    }
//    else
//    {
//      //
//      //Look left
//      //
//      N = FindProjection(QN->Left);
//      if(N == 0)
//      {
//        //
//        //Ok, now look right
//        //
//        N = FindProjection(QN->Right);
//      }
//    }
//  }
//  ALV_OUT();
//  return N;
//}
//
////******************************************************************************
//// PushProjections (private) 
////******************************************************************************
//// Purpose
//// This method looks for projections and pushes them down the tree to nodes
//// that contain the relations specified.
//// Warning: This is a RECURSIVE method!
////
//// Parameters
//// query_node *QN -- the node to operate on
//// query_node *pNode -- the node containing the projection attributes
////
//// Returns
//// N/A
////
//// Error Handling
//// None
////
//// Implementation Notes
//// None
////******************************************************************************
//void Query_tree::PushProjections(query_node *QN, query_node *pNode)
//{
//  ALV_IN("PushProjections(query_node *QN, query_node *pNode)");
//  Attribute::AttrStruct   a;
//  int         i;
//  int         j;
//
//  if((QN != 0) && (pNode != 0))
//  {
//    //
//    //Ignore the same node (hehe--now, that's a bug!)
//    //
//    if((QN->NodeId != pNode->NodeId) &&
//       (QN->NodeType == qntProject))
//    {
//      //
//      //If this node is a projection and has the table,
//      //add the attribute to QN and remove the attribute
//      //from pNode.
//      //
//      i = 0;
//      j = QN->Attributes->NumAttributes();
//      while(i < j)
//      {
//        a = QN->Attributes->GetAttribute(i);
//        if(HasRelation(QN, a.Table))
//        {
//          if(!HasAttribute(QN, a))
//          {
//            InsertAttribute(QN, a);
//          }
//          DelAttribute(pNode, a);
//        }
//        i++;
//      }
//    }
//    //
//    //Now traverse the tree if pNode still has attributes
//    //
//    if(pNode->Attributes->NumAttributes() != 0)
//    {
//      PushProjections(QN->Left, pNode);
//      PushProjections(QN->Right, pNode);
//    }
//  }
//  ALV_OUT();
//}
//
////******************************************************************************
//// DelAttribute (private) 
////******************************************************************************
//// Purpose
//// This method removes the attribute specified.
////
//// Parameters
//// query_node *QN -- the node to operate on
//// AttrStruct a -- the attribute to remove
////
//// Returns
//// N/A
////
//// Error Handling
//// None
////
//// Implementation Notes
//// None
////******************************************************************************
//void Query_tree::DelAttribute(query_node *QN, Attribute::AttrStruct a)
//{
//  ALV_IN("DelAttribute(query_node *QN, Attribute::AttrStruct a)");
//  if(QN != 0)
//  {
//    QN->Attributes->RemoveAttribute(a.Table, a.Value);
//  }
//  ALV_OUT();
//}
//
////******************************************************************************
//// HasRelation (private) 
////******************************************************************************
//// Purpose
//// This method returns TRUE if the node contains the relation specified.
////
//// Parameters
//// query_node *QN -- the node to operate on
//// char *Table -- the relation you're looking for
////
//// Returns
//// bool -- true = table found.
////
//// Error Handling
//// None
////
//// Implementation Notes
//// None
////******************************************************************************
//bool Query_tree::HasRelation(query_node *QN, char *Table)
//{
//  ALV_IN("HasRelation(query_node *QN, char *Table)");
//  bool    Found = false;
//  int     i;
//
//  i = 0;
//  if(QN != 0)
//  {
//    while(QN->Relations[i] != 0)
//    {
//      if(strcmp(QN->Relations[i]->GetTableName(), Table) == 0)
//      {
//        Found = true;
//      }
//      i++;
//    }
//  }
//  ALV_OUT();
//  return Found;
//}
//
////******************************************************************************
//// HasAttribute (private) 
////******************************************************************************
//// Purpose
//// This method returns TRUE if the node contains the Attribute specified.
////
//// Parameters
//// query_node *QN -- the node to operate on
//// AttrStruct a -- the Attribute you're looking for
////
//// Returns
//// bool -- true = attribute found.
////
//// Error Handling
//// None
////
//// Implementation Notes
//// None
////******************************************************************************
//bool Query_tree::HasAttribute(query_node *QN, Attribute::AttrStruct a)
//{
//  ALV_IN("HasAttribute(query_node *QN, Attribute::AttrStruct a)");
//  bool        Found = false;
//  int         i = -1;
//
//  if(QN != 0)
//  {
//    i = QN->Attributes->IndexOf(&a);
//    if(i >= 0)
//    {
//      Found = true;
//    }
//  }
//  ALV_OUT();
//  return Found;
//}
//
////******************************************************************************
//// IsLeaf (private) 
////******************************************************************************
//// Purpose
//// This method returns TRUE if the node specified is a leaf (no children).
////
//// Parameters
//// query_node *QN -- the node to operate on
////
//// Returns
//// bool -- true = node is a leaf.
////
//// Error Handling
//// None
////
//// Implementation Notes
//// None
////******************************************************************************
//bool Query_tree::IsLeaf(query_node *QN)
//{
//  ALV_IN("IsLeaf(query_node *QN)");
//  ALV_OUT();
//  return((QN->Left == 0) && (QN->Right == 0));
//}
//
////******************************************************************************
//// InsertAttribute (public) 
////******************************************************************************
//// Purpose
//// This method is used to place an attribute (c) in the query node (QN).
////
//// Parameters
//// query_node *QN -- the node to operate on
//// AttrStruct c -- the attribute to insert
////
//// Returns
//// N/A
////
//// Error Handling
//// None
////
//// Implementation Notes
//// None
////******************************************************************************
//void Query_tree::InsertAttribute(query_node *QN, Attribute::AttrStruct c)
//{
//  ALV_IN("InsertAttribute(query_node *QN, Attribute::AttrStruct c)");
//  if(QN != 0)
//  {
//    QN->Attributes->AddAttribute(-1, c.Table, c.Value, c.Type, 
//                   c.Width, c.Alias, c.Hide);
//  }
//  ALV_OUT();
//}
//
////******************************************************************************
//// InsertRelation (public) 
////******************************************************************************
//// Purpose
//// This method is used to place a relation (r) in the query node (QN).
////
//// Parameters
//// query_node *QN -- the node to operate on
//// Relation *r -- the table to insert
////
//// Returns
//// N/A
////
//// Error Handling
//// None
////
//// Implementation Notes
//// None
////******************************************************************************
//bool Query_tree::InsertRelation(query_node *QN, Relation *r)
//{
//  ALV_IN("InsertRelation(query_node *QN, Relation *r)");
//  bool        Found = false;
//  int         i;
//
//  if(QN != 0)
//  {
//    i = 0;
//    while(QN->Relations[i] != 0)
//    {
//      if(strcmp(QN->Relations[i]->GetTableName(), 
//            r->GetTableName()) == 0)
//      {
//        Found = true;
//      }
//      i++;
//    }
//    if(!Found)
//    {
//      QN->Relations[i] = r;
//    }
//  }
//  ALV_OUT();
//  return false;
//}
//
////******************************************************************************
//// FindRestriction (private) 
////******************************************************************************
//// Purpose
//// This method looks for a node containing a Restriction and returns the node
//// pointer.
//// Warning: This is a RECURSIVE method!
////
//// Parameters
//// query_node *QN -- the node to operate on
////
//// Returns
//// query_node * -- the node located or NULL for not found
////
//// Error Handling
//// None
////
//// Implementation Notes
//// This finds the first Restriction and is biased to the left tree.
////******************************************************************************
//Query_tree::query_node *Query_tree::FindRestriction(query_node *QN)
//{
//  ALV_IN("FindRestriction(query_node *QN)");
//  query_node   *N;
//
//  N = 0;
//  //
//  //Look for the first node that  
//  //has a where clause and return it.
//  //
//  if(QN != 0)
//  {
////
////CAB: Changed on 11/4/2005 due to change in EXPR and BuildALVQuery_tree
////
////    if((QN->NodeType == qntRestrict) &&
////       (QN->where_expr != NULL))
//    if (QN->where_expr != NULL)
//    {
//      N = QN;
//    }
//    else
//    {
//      //
//      //Look left
//      //
//      N = FindRestriction(QN->Left);
//      if(N == 0)
//      {
//        //
//        //Ok, now look right
//        //
//        N = FindRestriction(QN->Right);
//      }
//    }
//  }
//  ALV_OUT();
//  return N;
//}
//
////******************************************************************************
//// PushRestrictions (private) 
////******************************************************************************
//// Purpose
//// This method looks for Restrictions and pushes them down the tree to nodes
//// that contain the relations specified.
//// Warning: This is a RECURSIVE method!
////
//// Parameters
//// query_node *QN -- the node to operate on
//// query_node *pNode -- the node containing the Restriction attributes
////
//// Returns
//// N/A
////
//// Error Handling
//// None
////
//// Implementation Notes
//// None
////******************************************************************************
//void Query_tree::PushRestrictions(query_node *QN, query_node *pNode)
//{
//  ALV_IN("PushRestrictions(query_node *QN, query_node *pNode)");
//  query_node       *NewQN;
//
//  if((QN != 0) && (pNode != 0) && (pNode->Left != 0))
//  {
//        //
//        //(11/7/2005) CAB:
//        //
//        //Must push restrictions down the tree.
//        //
//        //Conditions:
//        //  1) QN is a join node
//        //  2) QN is a project node
//        //  3) QN is a restrict node 
//        //  4) All other nodes types are ignored.
//        //
//        //Methods:
//        //  1) if join or project and the children are not already restrictions
//        //     add a new node and put where clause in new node else
//        //     see if you can combine the child node and this one
//        //  2) if the node has the table and it is a join,
//    //     create a new node below it and push the restriction
//    //     to that node.
//    //  4) if the node is a restriction and has the table,
//    //     just add the expression to the node's expression list
//    //
//    //Ignore the same node (hehe--now, that's a bug!)
//    //
//        //
//        //Some other conditions to consider:
//        // 1) must traverse left tree to see if table comes from there
//        // 2) if it does, we're OK to insert this node
//        // 3) else don't insert!
//        //
//        if((QN->NodeId != pNode->NodeId) && (QN->NodeType == qntProject))
//    {
//            if (QN->Left != 0)
//            {
//        QN->Left = new query_node;
//        NewQN = QN->Left;
//          NewQN->Left = 0;
//            }
//            else
//            {
//                NewQN = QN->Left;
//        QN->Left = new query_node;
//        QN->Left->Left = NewQN;
//                NewQN = QN->Left;
//            }
//      NewQN->Cost = 0;
//      NewQN->Size = 0;
//      NewQN->SubQuery = 0;
//      NewQN->JoinCondition = (join_con_type) 0;
//      NewQN->JoinType = (type_join) 0;
//      NewQN->Right = 0;
//      for(long i = 0; i < MAXNODETABLES; i++)
//      {
//        NewQN->Relations[i] = 0;
//      }
//      NewQN->NodeId = QN->NodeId + 1;
//      NewQN->ParentNodeId = QN->NodeId;
//      NewQN->NodeType = qntRestrict;
//      NewQN->Attributes = new Attribute();
//            //
//            //Assign where_expr to reduction of pNode->where_expr with 
//            //attributes from Relation[0]
//            //
//            //But first, see if the relations are the same!
//            //
//        NewQN->where_expr = pNode->where_expr->GetReducedExpression(pNode->Relations[0]->GetAttributes());
//            if ((QN->Relations[0] != NULL) && (QN->Relations[0] == pNode->Relations[0]))
//            {
//                if (FindTableinExpr(pNode->where_expr, QN->Relations[0]->GetTableName()))
//                {
//                    //
//                    //This node has the relation. Push it down.
//                    //
//                    NewQN->Relations[0] = QN->Relations[0];
//                    QN->Relations[0] = 0;
//                }
//            }
//            else 
//            {
//                if (FindTableinTree(QN->Left, pNode->Relations[0]->GetTableName()))
//                {
//                    //
//                    //The relation is in the subtree
//                    //
//                    NewQN->Relations[0] = 0;
//                }
//            }
//            delete pNode->where_expr;
//            pNode->Relations[0] = 0;
//    }
//        else if((QN->NodeId != pNode->NodeId) && (QN->NodeType == qntRestrict))
//    {
//            //
//            //(11/7/2005) CAB: omitted due to no way to concatenate Expr classes
//            //
//            /*
//            bool SameRels = false;
//            //
//            //First, check to see if the relations are the same...
//            //
//            if (QN->Relations[0] == pNode->Relations[0])
//            {
//                SameRels = true;
//            }
//            else if (pNode->Relations[0] != NULL)
//            {
//                if (FindTableinTree(QN->Left, pNode->Relations[0]->GetTableName()))
//                {
//                    SameRels = true;
//                }
//            }
//            //
//            //Can combine ONLY if relations are the same
//            //
//            if (SameRels)
//            {
//                ((Expr::Expr *)pNode->where_expr)->Append(QN->where_expr);
//                QN->where_expr = pNode->where_expr->GetReducedExpression(pNode->Relations[0]->GetAttributes());
//                delete pNode->where_expr;
//                pNode->Relations[0] = 0;
//            }
//            */
//    }
//    else if((QN->NodeId != pNode->NodeId) && 
//        ((QN->Left == 0) || (QN->Right == 0)) &&
//        (QN->NodeType == qntJoin))
//    {
//      //
//      //Found a join that has one child. 
//      //Now add new node and move the
//      //expression down. There can only be 2 relations...
//      //
//      //
//      //if left child is null and left relation is the ltable
//      //in the expression, move expression to a new node 
//      //beneath this node.
//      //
//      if(QN->Relations[0] != 0)
//      {
//        QN->Left = new query_node;
//        NewQN = QN->Left;
//        NewQN->Cost = 0;
//        NewQN->Size = 0;
//        NewQN->SubQuery = 0;
//        NewQN->JoinCondition = (join_con_type) 0;
//        NewQN->JoinType = (type_join) 0;
//        NewQN->Left = 0;
//        NewQN->Right = 0;
//        for(long i = 0; i < MAXNODETABLES; i++)
//        {
//          NewQN->Relations[i] = 0;
//          //              NewQN->SelIndex[i] = 0;
//        }
//        NewQN->NodeId = QN->NodeId + 1;
//        NewQN->ParentNodeId = QN->NodeId;
//        NewQN->NodeType = qntRestrict;
//        NewQN->Attributes = new Attribute();
//        NewQN->Relations[0] = QN->Relations[0];
//        QN->Relations[0] = 0;
//        // Assign where_expr to reduction of pNode->where_expr with attributes from Relation[0]
//        NewQN->where_expr = pNode->where_expr->GetReducedExpression(NewQN->Relations[0]->GetAttributes());
//      }
//      //
//      //if right child is null and right relation is the ltable
//      //in the expression, move expression to a new node 
//      //beneath this node.
//      //
//      else if(QN->Relations[1] != 0)
//      {
//        QN->Right = new query_node;
//        NewQN = QN->Left;
//        NewQN->Cost = 0;
//        NewQN->Size = 0;
//        NewQN->SubQuery = 0;
//        NewQN->JoinCondition = (join_con_type) 0;
//        NewQN->JoinType = (type_join) 0;
//        NewQN->Left = 0;
//        NewQN->Right = 0;
//        for(long i = 0; i < MAXNODETABLES; i++)
//        {
//          NewQN->Relations[i] = 0;
//          //              NewQN->SelIndex[i] = 0;
//        }
//        NewQN->NodeId = QN->NodeId + 1;
//        NewQN->ParentNodeId = QN->NodeId;
//        NewQN->NodeType = qntRestrict;
//        NewQN->Attributes = new Attribute();
//        NewQN->Relations[0] = QN->Relations[1];
//        QN->Relations[1] = 0;
//        // Assign where_expr to reduction of pNode->where_expr with attributes from Relation[0]
//        NewQN->where_expr = pNode->where_expr->GetReducedExpression(NewQN->Relations[0]->GetAttributes());
//      }
//    }
//    //
//    //Now traverse the tree if pNode still has expressions
//    //
//    PushRestrictions(QN->Left, pNode);
//    PushRestrictions(QN->Right, pNode);
//  }
//  ALV_OUT();
//}
//
////******************************************************************************
//// IsJoinCondition (private) 
////******************************************************************************
//// Purpose
//// This method returns TRUE if the expression is a join condition -- it has
//// a table for both the left and right parameters.
////
//// Parameters
//// ExprStruct *e -- the expression to examine.
////
//// Returns
//// bool -- true = expression is a join condition.
////
//// Error Handling
//// None
////
//// Implementation Notes
//// None
////******************************************************************************
//bool Query_tree::IsJoinCondition(Expression::ExprStruct *e)
//{
//  ALV_IN("IsJoinCondition(Expression::ExprStruct *e)");
//  ALV_OUT();
//  return((strlen(e->lTable) != 0) && (strlen(e->rTable) != 0));
//}
//
////******************************************************************************
//// HasExpression (private) 
////******************************************************************************
//// Purpose
//// This method returns TRUE if the node contains the Expression specified.
////
//// Parameters
//// query_node *QN -- the node to operate on
//// ExprStruct *a -- the Expression you're looking for
////
//// Returns
//// bool -- true = Expression found.
////
//// Error Handling
//// None
////
//// Implementation Notes
//// None
////******************************************************************************
///*
//bool Query_tree::HasExpression(query_node *QN, Expression::ExprStruct *a)
//{
//  ALV_IN("HasExpression(query_node *QN, Expression::ExprStruct *a)");
//  ALV_OUT();
//  return false; //(QN->Expressions->IndexOf(*a) > -1);
//}
//*/
//
////******************************************************************************
//// DelRelation (private) 
////******************************************************************************
//// Purpose
//// This method removes the relation specified.
////
//// Parameters
//// query_node *QN -- the node to operate on
//// Relation *r -- the relation to remove
////
//// Returns
//// N/A
////
//// Error Handling
//// None
////
//// Implementation Notes
//// None
////******************************************************************************
//void Query_tree::DelRelation(query_node *QN, Relation *r)
//{
//  ALV_IN("DelRelation(query_node *QN, Relation *r)");
//  bool    Found = false;
//  int     i;
//
//  if(QN != 0)
//  {
//    i = 0;
//    while(!Found && (QN->Relations[i] != 0) && (i < MAXNODETABLES))
//    {
//      if(strcmp(QN->Relations[i]->GetTableName(), r->GetTableName()) == 0)
//      {
//        Found = true;
//        delete QN->Relations[i];
//      }
//      i++;
//    }
//  }
//  ALV_OUT();
//}
//
////******************************************************************************
//// FindNaturalJoin (private) 
////******************************************************************************
//// Purpose
//// This method looks for a node containing a theta join and returns the 
//// node pointer.
////
//// Parameters
//// query_node *QN -- the node to operate on
////
//// Returns
//// query_node * -- the node located or NULL for not found
////
//// Error Handling
//// None
////
//// Implementation Notes
//// None
////******************************************************************************
//Query_tree::query_node *Query_tree::FindNaturalJoin(query_node *QN)
//{
//  ALV_IN("FindNaturalJoin(query_node *QN)");
//  query_node               *N;
//  N = 0;
//
//  //
//  //Look for the first node that is a restriction that 
//  //has an equality involving two tables, which would be a join condition, stored in join_expr
//  //
//  if(QN != 0)
//  {
//    // ALVTODO:  the logic here seems redundant, it's basically checking for qntRestrict, that's all.
//    if(((QN->NodeType == qntRestrict) ||
//      (QN->NodeType == qntRestrict)) && (QN->join_expr != NULL))
//    {
//      N = QN;
//    }
//    else
//    {
//      //
//      //Look left
//      //
//      N = FindNaturalJoin(QN->Left);
//      if(N == 0)
//      {
//        //
//        //Ok, now look right
//        //
//        N = FindNaturalJoin(QN->Right);
//      }
//    }
//  }
//  ALV_OUT();
//
//  return N;
//}
//
////******************************************************************************
//// PushNaturalJoins (private) 
////******************************************************************************
//// Purpose
//// This method looks for theta joins and pushes them down the tree to the 
//// parent of two nodes that contain the relations specified.
//// Warning: This is a RECURSIVE method!
////
//// Parameters
//// query_node *QN -- the node to operate on
//// query_node *pNode -- the node containing the join
////
//// Returns
//// N/A
////
//// Error Handling
//// None
////
//// Implementation Notes
//// None
////******************************************************************************
//void Query_tree::PushNaturalJoins(query_node *QN, query_node *pNode)
//{
//  ALV_IN("PushNaturalJoins(query_node *QN, query_node *pNode)");
//  
//  // Now we must find the two tables that this join involves by searching the join_expr for Field nodes.
//  // We will store the results here:
//  Expr::Field lField("", "");
//  Expr::Field rField("", "");
//  
//  Expr::ExprList fieldlist;
//  if(pNode->join_expr)
//    pNode->join_expr->FindNodesOfType(Expr::ENT_FIELD, fieldlist);
//  
//  // ALVTODO:  See if this works all the time.
//  // For now we will just assume that the first 2 fields found are the left and right.  This might not 
//  // always be the case though. There could be more than 2 if the condition is something like a.x + a.y = b.x, 
//  // so what we really want is the first two fields with unique table names.  But, that wouldn't be a 'natural join', I think.
//  if(fieldlist.Length() >= 2)
//  {
//    Expr::ExprList::ExprListNode *node = fieldlist.m_head;
//    lField = *(Expr::Field *)node->expr;
//    node = node->next;
//    rField = *(Expr::Field *)node->expr;
//  }
//
//  if((QN != NULL) && (pNode != NULL) && (pNode->join_expr != NULL))
//  {
//    //
//    //Ignore the same node (hehe--now, that's a bug!)
//    //
//    //
//    //First, find the expression that is the join condition.
//    //
//    //
//    //If this node <> pnode &&
//    //this is a join node &&
//    //it doesn't already have the expression &&
//    //the children each have one of the tables
//    //move the expression here.
//    //
//    if((QN->NodeId != pNode->NodeId) &&
//       (QN->NodeType == qntJoin) &&
//        QN->join_expr == NULL &&
//       ((HasRelation(QN->Left, lField.m_table) && 
//       HasRelation(QN->Right, rField.m_table)) ||
//      (HasRelation(QN->Left, rField.m_table) && 
//       HasRelation(QN->Right, lField.m_table))))
//    {
//      //
//      //((if there is a join condition)...and (not same node) and
//      //(is a join that has the same tables) and 
//      //doesn't already have expression
//      //
//
//      //
//      //Add expression to node.
//      //Change join type to Natural Join.
//      //Remove expression from host node    
//      //
//      QN->join_expr = pNode->join_expr;
//      pNode->join_expr = NULL;
//      QN->JoinType = jnINNER;
//      //if(i > 1)
//      //{
//      // ALVTODO:  Figure out what this means and if it applies here.  It may just indicate if the join is 1 field OP field condition only.
//      //  QN->JoinCondition = jcUS;
//      //}
//      //else
//      //{
//      QN->JoinCondition = jcON;
//      //}
//    }
//
//    //
//    //Now traverse the tree if pNode still has expressions
//    //
//    PushNaturalJoins(QN->Left, pNode);
//    PushNaturalJoins(QN->Right, pNode);
//  }
//  ALV_OUT();
//}
//
////******************************************************************************
//// GetJoinCondition (private) 
////******************************************************************************
//// Purpose
//// This method returns the join expression from the node.
////
//// Parameters
//// query_node *QN -- the node to operate on
////
//// Returns
//// ExprStruct * -- the expression containing the join condition
////
//// Error Handling
//// None
////
//// Implementation Notes
//// None
////******************************************************************************
///*
//Expression::ExprStruct *Query_tree::GetJoinCondition(query_node *QN)
//{
//  ALV_IN("GetJoinCondition(query_node *QN)");
//  Expression::ExprStruct  *ex = 0;
//  bool        Found = false;
//
//  i = 0;
//  j = QN->Expressions->NumExpressions();
//  while((i < j) && !Found)
//  {
//    ex = QN->Expressions->GetExpression(i);
//    if((strlen(ex->lTable) > 0) && (strlen(ex->rTable) > 0) &&
//       (ex->Operation == 0))   // 0 = opEQ
//    {
//      Found = true;
//    }
//    else
//    {
//      ex = 0;
//    }
//    i++;
//  }
//  ALV_OUT();
//  return ex;
//}
//*/
//
////******************************************************************************
//// SetDatabase (public) 
////******************************************************************************
//// Purpose
//// This method sets the default database name for the query.
////
//// Parameters
//// char *db -- the database file path
////
//// Returns
//// N/A
////
//// Error Handling
//// None
////
//// Implementation Notes
//// None
////******************************************************************************
//void Query_tree::SetDatabase(char *db)
//{
//  ALV_IN("SetDatabase(char *db)");
//  if(Database != 0)
//  {
//    delete Database;
//  }
//  Database = strdup(db);
//  ALV_OUT();
//}
//
////******************************************************************************
//// GetDatabase (public) 
////******************************************************************************
//// Purpose
//// This method returns the current default database for the query tree.
////
//// Parameters
//// None
////
//// Returns
//// char *db -- the database file path
////
//// Error Handling
//// None
////
//// Implementation Notes
//// None
////******************************************************************************
//char *Query_tree::GetDatabase()
//{
//  ALV_IN("GetDatabase()");
//  ALV_OUT();
//  return Database;
//}
//
////******************************************************************************
//// BalanceJoins (private) 
////******************************************************************************
//// Purpose
//// This method will balance the joins once cost-based factors are applied.
////
//// Parameters
//// query_node *QN -- the node to operate on
////
//// Returns
//// N/A
////
//// Error Handling
//// None
////
//// Implementation Notes
//// NOTE: This method is not completed yet!
////******************************************************************************
//void Query_tree::BalanceJoins(query_node *QN)
//{
//  ALV_IN("BalanceJoins(query_node *QN)");
//  puts("YOU HAVEN'T DONE THIS STEP YET!!!\n");
//  ALV_OUT();
//}
//
////******************************************************************************
//// COptimization (public) 
////******************************************************************************
//// Purpose
//// This method performs cost-based optimization on the query tree. The 
//// operation is nondestructive.
////
//// Parameters
//// query_node *QN -- the node to operate on
////
//// Returns
//// N/A
////
//// Error Handling
//// None
////
//// Implementation Notes
//// NOTE: This method is not completed yet!
////******************************************************************************
//void Query_tree::COptimization()
//{
//  ALV_IN("COptimization()");
//  COpt = true;
//
//  //  BalanceJoins(Root);
//
//  //
//  //Search the tree looking for all possible indexes that can be used 
//  //and apply them to the SelIndex variable(s);
//  //
//  ApplyIndexes(Root);
//  ALV_OUT();
//}
//
////******************************************************************************
//// ApplyIndexes (private) 
////******************************************************************************
//// Purpose
//// This method will look for the best index for each node.
////
//// Parameters
//// query_node *QN -- the node to operate on
////
//// Returns
//// N/A
////
//// Error Handling
//// None
////
//// Implementation Notes
//// NOTE: This method is not completed yet!
////******************************************************************************
//void Query_tree::ApplyIndexes(query_node *QN)
//{
//  ALV_IN("ApplyIndexes(query_node *QN)");
//  ALV_OUT();
//  return;
//  /*
//    bool  Done = false;
//    int    i = 0;
//    int    j = 0;
//    Index  *Ndx;
//    char  *Str;
//    int    k = 0;
//  
//    if (QN != 0)
//    {
//      Str = new char[256];
//      //
//      //You need only apply indexes to leaves of the tree!
//      //So just look at nodes that have one or more children.
//      //
//      switch (QN->NodeType)
//      {
//        case qntProject:
//        case qntRestrict:
//        {
//          if ((QN->Left == 0) && (QN->Relations[0] != 0))
//          {
//            //
//            //Find out what is available...
//            //
//            Ndx = new Index(QN->Relations[0]->GetDatabaseName(), 
//                    QN->Relations[0]->GetTableName());
//            Ndx->SetPath(SetPath);
//            Ndx->Open();
//            if (Ndx->GetNumIndexes() > 0)
//            {
//              QN->SelIndex[0] = Ndx;
//            }
//            else
//            {
//              delete Ndx;
//            }
//          }
//          break;
//        }
//        case qntJoin:
//        {
//          if ((QN->Left == 0) && (QN->Relations[0] != 0))
//          {
//            //
//            //Find out what is available...
//            //
//            Ndx = new Index(QN->Relations[0]->GetDatabaseName(), 
//                    QN->Relations[0]->GetTableName());
//            Ndx->SetPath(SetPath);
//            Ndx->Open();
//            if (Ndx->GetNumIndexes() > 0)
//            {
//              QN->SelIndex[0] = Ndx;
//            }
//            else
//            {
//              delete Ndx;
//            }
//          }
//          if ((QN->Right == 0) && (QN->Relations[1] != 0))
//          {
//            //
//            //Find out what is available...
//            //
//            Ndx = new Index(QN->Relations[1]->GetDatabaseName(), 
//                    QN->Relations[1]->GetTableName());
//            Ndx->SetPath(SetPath);
//            Ndx->Open();
//            if (Ndx->GetNumIndexes() > 0)
//            {
//              QN->SelIndex[1] = Ndx;
//            }
//            else
//            {
//              delete Ndx;
//            }
//          }
//          break;
//        }
//      }
//      delete Str;
//      ApplyIndexes(QN->Left);
//      ApplyIndexes(QN->Right);
//    }
//  */
//  ALV_OUT();
//}
//
////******************************************************************************
//// SplitRestrictWithProject (private) 
////******************************************************************************
//// Purpose
//// This method looks for restrictions that have attributes (thus are both
//// projections and restrictions) and breaks them into two nodes.
//// Warning: This is a RECURSIVE method!
////
//// Parameters
//// query_node *QN -- the node to operate on
////
//// Returns
//// N/A
////
//// Error Handling
//// None
////
//// Implementation Notes
//// **** WARNING! This is a VERY complex algorithm! ****
////******************************************************************************
//void Query_tree::SplitRestrictWithProject(query_node *QN)
//{
//  ALV_IN("SplitRestrictWithProject(query_node *QN)");
//  if(QN != 0)
//  {
//    //
//    //if this restriction node has both attributes and expressions
//    //it is a node that is both a restriction and a projection.
//    //
//    // ---or---
//    //
//    //if this projection node has both attributes and expressions
//    //it is a node that is both a restriction and a projection.
//    //
//    if(((QN->Attributes->NumAttributes() > 0) &&
//      (QN->where_expr != NULL)) &&
//       ((QN->NodeType == qntProject) || (QN->NodeType == qntRestrict)))
////
////CAB: Changed this on 11/4/2005 because the BuildQuery_tree changed
////node type to JOIN when a join clause was found.
////
////      || (QN->NodeType == qntJoin)))
//    {
//      //Create a new node and:
//      // 1) Move the expressions to the new node.
//      // 2) Set the new node's children = current node children
//      // 3) Set the new node's relations = current node relations.
//      // 4) Set current node's left child = new node;
//      // 5) Set new node's id = current id + 1000;
//      // 6) set parent id, etc.
//      //
//      query_node *NewNode = new query_node;
//      NewNode->Child = "L";
//      NewNode->NodeType = qntRestrict;
//      // ALVTODO: This doesn't seem right, should it be QN->NodeType == qntJoin?
//      if(NewNode->NodeType == qntJoin)
//      {
//        NewNode->JoinCondition = QN->JoinCondition;
//        NewNode->JoinType = QN->JoinType;
//      }
//      QN->NodeType = qntProject;
//      NewNode->Attributes = new Attribute();
//      NewNode->where_expr = QN->where_expr;
//      QN->where_expr = NULL;
//      NewNode->Left = QN->Left;
//      NewNode->Right = QN->Right;
//      NewNode->ParentNodeId = QN->NodeId;
//      NewNode->NodeId = QN->NodeId + 1000;
//      if(NewNode->Left)
//      {
//        NewNode->Left->ParentNodeId = NewNode->NodeId;
//      }
//      if(NewNode->Right)
//      {
//        NewNode->Right->ParentNodeId = NewNode->NodeId;
//      }
//      for(int i = 0; i < MAXNODETABLES; i++)
//      {
//        NewNode->Relations[i] = QN->Relations[i];
//        QN->Relations[i] = NULL;
//      }
//      QN->Left = NewNode;
//      QN->Right = 0;
//    }
//    SplitRestrictWithProject(QN->Left);
//    SplitRestrictWithProject(QN->Right);
//  }
//  ALV_OUT();
//}
//
////******************************************************************************
//// SplitRestrictWithJoin (private) 
////******************************************************************************
//// Purpose
//// This method looks for Joins that have where expressions (thus are both
//// joins and restrictions) and breaks them into two nodes.
//// Warning: This is a RECURSIVE method!
////
//// Parameters
//// query_node *QN -- the node to operate on
////
//// Returns
//// N/A
////
//// Error Handling
//// None
////
//// Implementation Notes
//// **** WARNING! This is a VERY complex algorithm! ****
////******************************************************************************
//void Query_tree::SplitRestrictWithJoin(query_node *QN)
//{
//    int j = 0;
//
//  ALV_IN("SplitRestrictWithJoin(query_node *QN)");
//  if(QN != 0)
//  {
//    //
//    //if this join node has both join and where expressions
//    //it is a node that is both a join and a restriction.
//    //
//    // ---or---
//    //
//    //if this retriction node has both join and where expressions
//    //it is a node that is both a restriction and a join.
//    //
//    if(((QN->join_expr != NULL) &&
//      (QN->where_expr != NULL)) &&
//       ((QN->NodeType == qntJoin) || (QN->NodeType == qntRestrict)))
//    {
//            bool isLeft = true;
//
//      //Create a new node and:
//      // 1) Move the where expressions to the new node.
//      // 2) Set the new node's children = current node children
//      // 3) Set the new node's relations = current node relations.
//      // 4) Set current node's left or right child = new node;
//      // 5) Set new node's id = current id + 200;
//      // 6) set parent id, etc.
//            // 7) determine which table needs to be used for the 
//            //    restrict node.
//      //
//      query_node *NewNode = new query_node;
//      NewNode->NodeType = qntRestrict;
//      NewNode->ParentNodeId = QN->NodeId;
//      NewNode->NodeId = QN->NodeId + 200;
//      NewNode->where_expr = QN->where_expr;
//      QN->where_expr = NULL;
//            
//      for(int i = 0; i < MAXNODETABLES; i++)
//      {
//                if (QN->Relations[i] != NULL)
//                {
//                    if (FindTableinExpr(NewNode->where_expr, QN->Relations[i]->GetTableName()))
//                    {
//                        NewNode->Relations[j] = QN->Relations[i];
//                        j++;
//                        if (i != 0)
//                        {
//                            isLeft = false;
//                        }
//                QN->Relations[i] = NULL;
//                    }
//                }
//      }
//        NewNode->Right = 0;
//            if (isLeft)
//            {
//                NewNode->Child = "L";
//          NewNode->Left = QN->Left;
//          QN->Left = NewNode;
//            }
//            else
//            {
//                NewNode->Child = "R";
//          NewNode->Left = QN->Right;
//          QN->Right = NewNode;
//            }
//            if (NewNode->Left)
//            {
//          NewNode->Left->ParentNodeId = NewNode->NodeId;
//            }
//            j = QN->Attributes->NumAttributes();
//            //
//            // j > 0 indicates we have a projection in here too!
//            // so we need to add the join attributes to the new node
//            // and leave any that aren't in the list of attributes 
//            // for the new node.
//            //
//            if ((QN->NodeType == qntJoin) && (j > 0))
//            {
//                Attribute *attribs = 0;
//                Attribute::AttrStruct attr;
//                int ii = 0;
//                int jj = 0;
//
//                //
//                // You need to add the attributes from this 
//                // node to the newnode that match the table in 
//                // the new node.
//                //
//                // You also need to add the attributes for the
//                // join condition!
//                //
//                //
//                // BUT!! If the attributes in the node is a '*'
//                // just copy them to the new node.
//                //
//                if ((QN->Attributes->NumAttributes() == 1) &&
//                    (stricmp("*", QN->Attributes->GetAttribute(0).Value) == 0))
//                {
//                    NewNode->Attributes = new Attribute();
//                    NewNode->Attributes->AddAttribute(j, &QN->Attributes->GetAttribute(0));
//                }
//                else
//                {
//                    attribs = NewNode->Relations[0]->GetAttributes();
//                    j = attribs->NumAttributes();
//                    //
//                    // Loop through all of the available attributes
//                    // and add them to the projection
//                    //
//                    NewNode->Attributes = new Attribute();
//                    for (i = 0; i < j; i++)
//                    {
//                        attr = attribs->GetAttribute(i);
//                        //
//                        // if attr in list of projections, add it
//                        //
//                        jj = QN->Attributes->IndexOf(attr.Table, attr.Value);
//                        if (jj > -1)
//                        {
//                            NewNode->Attributes->AddAttribute(ii, &attr);
//                            ii++;
//                            QN->Attributes->RemoveAttribute(jj);
//                        }
//                        //
//                        // if attr in join expression, add it
//                        //
//                        else if (FindAttrinExpr(QN->join_expr, attr.Table, attr.Value)) 
//                        {
//                            attr.Hide = true;
//                            NewNode->Attributes->AddAttribute(ii, &attr);
//                            ii++;
//                        }
//                    }
//                }
//            }
//            else
//            {
//            QN->NodeType = qntJoin;
//                NewNode->Attributes = new Attribute();
//            }
//    }
//    SplitRestrictWithJoin(QN->Left);
//    SplitRestrictWithJoin(QN->Right);
//  }
//  ALV_OUT();
//}
//
////******************************************************************************
//// SplitProjectWithJoin (private) 
////******************************************************************************
//// Purpose
//// This method looks for Joins that have attributes (thus are both
//// joins and projections) and breaks them into two nodes.
//// Warning: This is a RECURSIVE method!
////
//// Parameters
//// query_node *QN -- the node to operate on
////
//// Returns
//// N/A
////
//// Error Handling
//// None
////
//// Implementation Notes
//// **** WARNING! This is a VERY complex algorithm! ****
////******************************************************************************
//void Query_tree::SplitProjectWithJoin(query_node *QN)
//{
//    int j = 0;
//
//  ALV_IN("SplitProjectWithJoin(query_node *QN)");
//  if(QN != 0)
//  {
//    //
//    //if this join node has both join and has attributes
//    //it is a node that is both a join and a projection.
//    //
//    // ---or---
//    //
//    //if this projection node has both join and attributes
//    //it is a node that is both a projection and a join.
//    //
//    if((QN->join_expr != NULL) &&
//       ((QN->NodeType == qntJoin) || (QN->NodeType == qntProject)))
//    {
//
//      //Create a new node and:
//      // 1) Move the where expressions to the new node.
//      // 2) Set the new node's children = current node children
//      // 3) Set the new node's relations = current node relations.
//      // 4) Set current node's left or right child = new node;
//      // 5) Set new node's id = current id + 300;
//      // 6) set parent id, etc.
//      //
//            //
//            //Need to determine which table needs a projection
//            //
//            QN->NodeType = qntJoin;
//            if (QN->Left == 0)
//            {
//                //
//                //Process left relation creating a new node
//                //
//          query_node *NewNode = new query_node;
//          NewNode->NodeType = qntProject;
//          NewNode->ParentNodeId = QN->NodeId;
//          NewNode->NodeId = QN->NodeId + 300;
//          for(int i = 0; i < MAXNODETABLES; i++)
//          {
//                    NewNode->Relations[0] = 0;
//                }
//                NewNode->Relations[0] = QN->Relations[0];
//                QN->Relations[0] = 0;
//                NewNode->Left = QN->Left;
//                QN->Left = NewNode;
//                NewNode->Right = 0;
//                NewNode->Child = "L";
//                if (NewNode->Left != 0)
//                {
//              NewNode->Left->ParentNodeId = NewNode->NodeId;
//                }
//                //
//                //Now move the attributes down.
//                //
//                j = QN->Attributes->NumAttributes();
//                NewNode->Attributes = new Attribute();
//                //
//                // j > 0 indicates we have a projection!
//                // so we need to add the join attributes to the new node
//                // and leave any that aren't in the list of attributes 
//                // for the new node.
//                //
//                //
//                // BUT!! If the attributes in the node is a '*'
//                // just copy them to the new node.
//                //
//                if ((j == 1) &&
//                    (stricmp("*", QN->Attributes->GetAttribute(0).Value) == 0))
//                {
//                    NewNode->Attributes = new Attribute();
//                    NewNode->Attributes->AddAttribute(j, &QN->Attributes->GetAttribute(0));
//                    if (QN->Right != 0)
//                    {
//                        QN->Attributes->RemoveAttribute(0);
//                    }
//                }
//                else if (j > 0)
//                {
//                    Attribute *attribs = 0;
//                    Attribute::AttrStruct attr;
//                    int ii = 0;
//                    int jj = 0;
//
//                    //
//                    // You need to add the attributes from this 
//                    // node to the newnode that match the table in 
//                    // the new node.
//                    //
//                    // You also need to add the attributes for the
//                    // join condition!
//                    //
//                    attribs = NewNode->Relations[0]->GetAttributes();
//                    j = attribs->NumAttributes();
//                    //
//                    // Loop through all of the available attributes
//                    // and add them to the projection
//                    //
//                    NewNode->Attributes = new Attribute();
//                    for (i = 0; i < j; i++)
//                    {
//                        attr = attribs->GetAttribute(i);
//                        //
//                        // if attr in list of projections, add it
//                        //
//                        jj = QN->Attributes->IndexOf(attr.Table, attr.Value);
//                        if (jj > -1)
//                        {
//                            NewNode->Attributes->AddAttribute(ii, &attr);
//                            ii++;
//                            QN->Attributes->RemoveAttribute(jj);
//                        }
//                        //
//                        // if attr in join expression, add it
//                        //
//                        else if (FindAttrinExpr(QN->join_expr, attr.Table, attr.Value)) 
//                        {
//                            attr.Hide = true;
//                            NewNode->Attributes->AddAttribute(ii, &attr);
//                            ii++;
//                        }
//                    }
//                }
//            }
//            if (QN->Right == 0)
//            {
//                //
//                //Process right relation creating a new node.
//                //
//          query_node *NewNode = new query_node;
//          NewNode->NodeType = qntProject;
//          NewNode->ParentNodeId = QN->NodeId;
//          NewNode->NodeId = QN->NodeId + 400;
//          for(int i = 0; i < MAXNODETABLES; i++)
//          {
//                    NewNode->Relations[0] = 0;
//                }
//                NewNode->Relations[0] = QN->Relations[1];
//                QN->Relations[1] = 0;
//                NewNode->Left = QN->Right;
//                QN->Right = NewNode;
//                NewNode->Right = 0;
//                NewNode->Child = "R";
//
//                if (NewNode->Left != 0)
//                {
//              NewNode->Left->ParentNodeId = NewNode->NodeId;
//                }
//                //
//                //Now move the attributes down.
//                //
//                j = QN->Attributes->NumAttributes();
//                NewNode->Attributes = new Attribute();
//                //
//                // j > 0 indicates we have a projection!
//                // so we need to add the join attributes to the new node
//                // and leave any that aren't in the list of attributes 
//                // for the new node.
//                //
//                if ((j == 1) &&
//                    (stricmp("*", QN->Attributes->GetAttribute(0).Value) == 0))
//                {
//                    NewNode->Attributes = new Attribute();
//                    NewNode->Attributes->AddAttribute(j, &QN->Attributes->GetAttribute(0));
//                    QN->Attributes->RemoveAttribute(0);
//                }
//                else if (j > 0)
//                {
//                    Attribute *attribs = 0;
//                    Attribute::AttrStruct attr;
//                    int ii = 0;
//                    int jj = 0;
//
//                    //
//                    // You need to add the attributes from this 
//                    // node to the newnode that match the table in 
//                    // the new node.
//                    //
//                    // You also need to add the attributes for the
//                    // join condition!
//                    //
//                    attribs = NewNode->Relations[0]->GetAttributes();
//                    j = attribs->NumAttributes();
//                    //
//                    // Loop through all of the available attributes
//                    // and add them to the projection
//                    //
//                    NewNode->Attributes = new Attribute();
//                    for (i = 0; i < j; i++)
//                    {
//                        attr = attribs->GetAttribute(i);
//                        //
//                        // if attr in list of projections, add it
//                        //
//                        jj = QN->Attributes->IndexOf(attr.Table, attr.Value);
//                        if (jj > -1)
//                        {
//                            NewNode->Attributes->AddAttribute(ii, &attr);
//                            ii++;
//                            QN->Attributes->RemoveAttribute(jj);
//                        }
//                        //
//                        // if attr in join expression, add it
//                        //
//                        else if (FindAttrinExpr(QN->join_expr, attr.Table, attr.Value)) 
//                        {
//                            attr.Hide = true;
//                            NewNode->Attributes->AddAttribute(ii, &attr);
//                            ii++;
//                        }
//                    }
//                }
//            }
//    }
//    SplitProjectWithJoin(QN->Left);
//    SplitProjectWithJoin(QN->Right);
//  }
//  ALV_OUT();
//}
//
//Attribute *Query_tree::GetAttribfromLeftTree(query_node *QN)
//{
//    if (QN != 0)
//    {
//        if (QN->Relations[0] != NULL)
//        {
//            return QN->Relations[0]->GetAttributes();
//        }
//        else
//        {
//            return GetAttribfromLeftTree(QN->Left);
//        }
//    }
//    return 0;
//}
//
//Attribute *Query_tree::GetAttribfromRightTree(query_node *QN)
//{
//    Attribute *a = 0;
//
//    if (QN != 0)
//    {
//        if (QN->Relations[0] != NULL)
//        {
//            a = QN->Relations[0]->GetAttributes();
//        }
//        else
//        {
//            a = GetAttribfromRightTree(QN->Right);
//        }
//    }
//    return a;
//}
//
//bool Query_tree::FindTableinTree(query_node *QN, char *tbl)
//{
//  ALV_IN("FindTableinTree(query_node *QN, char *tbl)");
//    bool Found = false;
//
//    if (QN != 0)
//  {
//        if (QN->where_expr != NULL)
//        {
//            if (FindTableinExpr(QN->where_expr, tbl))
//            {
//                Found = true;
//      }
//    }
//  }
//    if (!Found)
//    {
//        Found = FindTableinTree(QN->Left, tbl);
//    }
//    if (!Found)
//    {
//        Found = FindTableinTree(QN->Right, tbl);
//    }
//  ALV_OUT();
//  return Found;
//}
//
//bool Query_tree::FindTableinExpr(Expr::Expr *e, char *tbl)
//{
//  ALV_IN("FindTableinExpr(Expr::Expr *e, char *tbl)");
//  bool Found = false;
//
//  if (e)
//  {
//    Expr::ExprList equalsList;
//    e->FindNodesOfType(Expr::ENT_FIELD, equalsList);
//    for (Expr::ExprList::ExprListNode *node = equalsList.m_head; node != NULL; node = node->next)
//    {
//      Expr::Field *thisExpr = (Expr::Field *)node->expr;
//         char *fieldTable = ((Expr::Field *)thisExpr)->m_table;
//        if ((fieldTable != NULL) && (stricmp(fieldTable, tbl) == 0))
//        {
//          Found = true;
//            }
//    }
//    }
//    ALV_OUT();
//  return Found;
//}
//
//bool Query_tree::FindAttrinExpr(Expr::Expr *e, char *tbl, char *value)
//{
//  ALV_IN("FindAttrinExpr(Expr::Expr *e, char *tbl, char *value)");
//  bool Found = false;
//
//  if (e)
//  {
//    Expr::ExprList equalsList;
//    e->FindNodesOfType(Expr::ENT_FIELD, equalsList);
//    for (Expr::ExprList::ExprListNode *node = equalsList.m_head; node != NULL; node = node->next)
//    {
//      Expr::Field *thisExpr = (Expr::Field *)node->expr;
//         char *fieldTable = ((Expr::Field *)thisExpr)->m_table;
//            char *fieldValue = ((Expr::Field *)thisExpr)->m_field;
//        if ((fieldTable != NULL) && (fieldValue != NULL) &&
//                (stricmp(fieldTable, tbl) == 0) &&
//                (stricmp(fieldValue, value) == 0))
//        {
//          Found = true;
//            }
//    }
//    }
//    ALV_OUT();
//  return Found;
//}
//
//
////******************************************************************************
//// GetDistinct (public) 
////******************************************************************************
//// Purpose
//// This method returns the distinct select option setting.
////
//// Parameters
//// None
////
//// Returns
//// bool -- TRUE = do a DISTINCT option
////
//// Error Handling
//// None
////
//// Implementation Notes
//// None
////******************************************************************************
//bool Query_tree::GetDistinct()
//{
//  ALV_IN("GetDistinct()");
//  ALV_OUT();
//  return Distinct;
//}
//
////******************************************************************************
//// SetDistinct (public) 
////******************************************************************************
//// Purpose
//// This method sets the distinct select option to the bool passed. 
////
//// Parameters
//// bool Value -- TRUE = do a DISTINCT option
////
//// Returns
//// N/A
////
//// Error Handling
//// None
////
//// Implementation Notes
//// None
////******************************************************************************
//void Query_tree::SetDistinct(bool Value)
//{
//  ALV_IN("SetDistinct(bool Value)");
//  Distinct = Value;
//  ALV_OUT();
//}
