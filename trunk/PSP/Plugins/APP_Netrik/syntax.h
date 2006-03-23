/*
   netrik -- The ANTRIK Internet Viewer
   Copyright (C) Olaf D. Buddenhagen AKA antrik, et al (see AUTHORS)
   Published under the GNU GPL; see LICENSE for details.
*/
/*
 * syntax.h -- declarations for HTML facilities (defined in facilites.c) and
 * anything else related to generating and processing the syntax tree.
 *
 * (C) 2001, 2002 antrik
 *     2002 Patrice Neff
 */
#ifndef __syntax_h
#define __syntax_h

#include "config.h"    /* DEBUG */
#include "load.h"
#include "items.h"

/* update facilities.c also! */
enum Element_type {
   EL_HTML, EL_HEAD, EL_BODY,
   EL_TITLE, EL_META, EL_STYLE, EL_SCRIPT,
   EL_H1, EL_H2, EL_H3, EL_H4, EL_H5, EL_H6,
   EL_P,
   EL_EM, EL_I,
   EL_STRONG, EL_B,
   EL_CENTER,
   EL_A,
   EL_BR,
   EL_PRE,
   EL_TABLE,
   EL_TR,
   EL_TD, EL_TH,
   EL_UL, EL_LI,
   EL_OL,
   EL_DL, EL_DT, EL_DD,
   EL_HR,
   EL_INS, EL_DEL,
   EL_U, EL_S, EL_STRIKE,
   EL_FORM, EL_INPUT, EL_SELECT, EL_BUTTON, EL_OPTION, EL_TEXTAREA,
   EL_IMG,
   EL_DIV, EL_SPAN,
   EL_NO,    /* must come after all normal elements (this one assigned, if nothing found in table) */
   EL_GLOBAL    /* must come after EL_NO (not to be searched in table) */
};

/* grouping of element types necessary for SGML (missing end tags) handling */
/* update facilities.c also! */
enum Element_group {
   GROUP_SINGLE,    /* no content allowed (single tags) */
   GROUP_OBLIGATE,    /* must have end tag */
   GROUP_HTML,
   GROUP_HEAD,    /* <head> and <body> */
   GROUP_LIST_ITEM,
   GROUP_TABLE_ROW,
   GROUP_TABLE_CELL,
   GROUP_OPTION,
   GROUP_PARAGRAPH
};

/* miscallenous information about all element types */
struct Element_data {
   char			name[8];    /* textual element name */
   int			breaks;    /* this element breaks a text block (beginns new line); 2=makes paragraph (enclosed between blank lines) */
   int			force_box;    /* always creates own box */
   enum Element_group	group;    /* to which group of element types it belongs */
   int			visible;    /* whether the content of this element should be rendered */
};

extern const struct Element_data	element_table[];

/* update facilities.c also! */
enum Attr_type {
   ATTR_NAME,
   ATTR_HREF,
   ATTR_TITLE,
   ATTR_COLSPAN,
   ATTR_ALIGN,
   ATTR_TYPE,
   ATTR_SIZE,
   ATTR_ALT,
   ATTR_VALUE,
   ATTR_COLS,
   ATTR_ROWS,
   ATTR_ID,
   ATTR_ACTION,
   ATTR_METHOD,
   ATTR_ENCTYPE,
   ATTR_CHECKED,
   ATTR_MULTIPLE,
   ATTR_SELECTED,
   ATTR_NO    /* must be last (this one assigned, if nothing found in table) */
};

/* miscallenous information about all attribute types */
struct Attr_data {
   char			name[8];    /* attribute name */
   int			numeric;    /* true if the element is to be treated as numeric */
   enum Element_type	el;      /* the element to which this attribute belongs. EL_NO if it doesn't matter. you must set this to something other than EL_NO if you have a default value. */
   char			def_val[5];      /* the default value. if this is set to a nonzero value it also means that the attribute is obligatory for this element */
};

extern const struct Attr_data	attr_table[];

/* all data for one element in parse tree */
struct Element {
   struct Element			*list_next;    /* next element in linear list (first child if any; sibling or first element of next branch otherwise) */
   struct Element			*parent;    /* the element in whose text area this one is contained */

   int					closed;    /* don't allow any more elements inside this one (helper flag for sgml_rework()) */

   union Element_name {
      char		*str;    /* string extracted from source */
      enum Element_type	type;    /* looked up type */
   } 				name;    /* kind of element */

   int				attr_count;    /* number of attributes */
   struct Attr {    /* complete data for one attribute */
      union Attr_name {
	 char	*str;
	 enum Attr_type
	 	type;	
      }			name;    /* kind of attribute */
      union Attr_value {
	 char	*str;
	 int	num;
      } 		value;    /* value */
   } 				*attr;    /* (dynamic) array of attribute structures */

   char				*content;    /* *parent's* content between end of last element and begining of this one (own content is stored in sub-elements) */
};

struct Ref {
   char			str[7];    /* reference name */
   unsigned char	replace;    /* replacement char */
};

extern const struct Ref		ref_table[];    /* known entity references */

enum Syntax_error {
   SE_NO=0,
   SE_BREAK,    /* user break during loading (not actually a syntax error, but can be neatly handled that way) */
   SE_DISCOURAGED,
   SE_UNIMPLEMENTED,
   SE_WORKAROUND,
   SE_CRITICAL,
   SE_FAIL,    /* error occured while reading file (not actually a syntax error, but can be neatly handled that way) */
   SE_NODATA    /* document contained no data (not actually a syntax error, but can be neatly handled that way) */
};


/* parse-syntax.c */
struct Element *parse_syntax(struct Resource *input, enum Syntax_error *err_level);    /* extract elements and content */
void free_syntax(struct Element *tree_top, int elements_parsed);    /* unallocate syntax tree */

/* dump-tree.c */
void dump_tree(struct Element *tree_top, int dump_content, int elements_parsed);    /* dump element hierarchy (and content, if "dump_content") */

/* parse-elements.c */
void parse_elements(struct Element *syntax_tree);    /* look up element and attribute names in table */

/* sgml.c */
#ifndef XHTML_ONLY
void sgml_rework(struct Element *tree_top);    /* fix broken syntax tree generated by missing end tags */
#endif

/* parse-struct.c */
struct Item *parse_struct(struct Element *syntax_tree);    /* create structure tree */
void free_items(struct Item *item_tree);    /* unallocate item tree */
struct Link_list *make_link_list(struct Item *item_tree);    /* create list of all links in the page */
void free_links(struct Link_list *list);    /* unallocate link list */
struct Anchor_list *make_anchor_list(struct Item *item_tree);    /* create list of all anchors in the page */
void free_anchors(struct Anchor_list *list);    /* unallocate anchor list */

#endif
