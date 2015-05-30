#ifndef GRAMMAR_HH
#define GRAMMAR_HH

#include <ostream>
#include "relmodel.hh"
#include <memory>

using std::shared_ptr;

struct prod {
  struct prod *parent;
  virtual void out(std::ostream &out) = 0;
};

struct table_ref : public prod {
  named_relation *t;
  static shared_ptr<table_ref> factory(scope &s);
  virtual ~table_ref() { }
  virtual std::string ident() { return t->ident(); }
};

struct table_or_query_name : public table_ref {
  virtual void out(std::ostream &out);
  table_or_query_name(scope &s);
  virtual ~table_or_query_name() { }
  static int sequence;
  virtual std::string ident() { return alias; }
  std::string alias;
};

struct table_subquery : public table_ref {
  virtual void out(std::ostream &out);
  struct query_spec *query;
  table_subquery(scope &s);
  static int instances;
  virtual ~table_subquery();
};

struct joined_table : table_ref {
  virtual void out(std::ostream &out);  
  joined_table(scope &s);
  std::string type;
  std::string condition;
  std::string alias;
  virtual std::string ident() { return alias; }
  shared_ptr<table_ref> lhs;
  shared_ptr<table_ref> rhs;
  virtual ~joined_table() {
  }
};

struct from_clause : public prod {
  std::vector<shared_ptr<table_ref> > reflist;
  virtual void out(std::ostream &out);
  from_clause(scope &s);
  ~from_clause() { }
};

struct value_expr: public prod {
  std::string type;
  virtual void out(std::ostream &out) = 0;
  virtual ~value_expr() { }
  static struct value_expr *factory(struct query_spec *q);
};

struct const_expr: value_expr {
  const_expr() { type = "integer"; }
  virtual void out(std::ostream &out) { out << random()%43; }
  virtual ~const_expr() { }
};

struct column_reference: value_expr {
  column_reference(struct query_spec *q);
  virtual void out(std::ostream &out) { out << reference; }
  std::string reference;
  virtual ~column_reference() { }
};

struct bool_expr : value_expr {
  virtual void out(std::ostream &out) { out << "true"; }
  virtual ~bool_expr() { }
  bool_expr(struct query_spec *q) { type = "bool"; }
};

struct comparison_op : bool_expr {
  value_expr *lhs;
  value_expr *rhs;
  op *oper;
  comparison_op(struct query_spec *q);
  virtual ~comparison_op() {
    delete lhs; delete rhs;
  };
  virtual void out(std::ostream &o) {
    lhs->out(o); o << oper->name; rhs->out(o);
  }
};
  
struct select_list : public prod {
  struct query_spec *query;
  std::vector<value_expr*> value_exprs;
  relation derived_table;
  int columns = 0;
  select_list(struct query_spec *q);
  virtual void out(std::ostream &out);
  ~select_list() { for (auto p : value_exprs) delete p; }
};

struct query_spec : public prod {
  std::string set_quantifier;
  from_clause fc;
  select_list sl;
  comparison_op search;
  std::string limit_clause;
  virtual void out(std::ostream &out);
  query_spec(scope &s);
  virtual ~query_spec() { }
};


#endif
