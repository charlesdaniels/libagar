with agar.core.tail_queue;
with agar.core.types;
with agar.gui.widget.menu;
with agar.gui.widget.scrollbar;

package agar.gui.widget.graph is

  use type c.unsigned;

  type vertex_t;
  type vertex_access_t is access all vertex_t;
  pragma convention (c, vertex_access_t);

  type edge_t;
  type edge_access_t is access all edge_t;
  pragma convention (c, edge_access_t);

  type graph_t;
  type graph_access_t is access all graph_t;
  pragma convention (c, graph_access_t);

  package vertex_tail_queue is new agar.core.tail_queue
    (entry_type => vertex_access_t);
  package edge_tail_queue is new agar.core.tail_queue
    (entry_type => edge_access_t);

  type vertex_style_t is (GRAPH_RECTANGLE, GRAPH_CIRCLE);
   for vertex_style_t use (GRAPH_RECTANGLE => 0, GRAPH_CIRCLE => 1);
   for vertex_style_t'size use c.unsigned'size;
  pragma convention (c, vertex_style_t);

  ndefcolors : constant := 16;
  label_max  : constant := 64;

  type vertex_label_t is array (1 .. label_max) of aliased c.char;
  pragma convention (c, vertex_label_t);

  subtype vertex_flags_t is c.unsigned;
  GRAPH_MOUSEOVER  : constant vertex_flags_t := 16#01#;
  GRAPH_SELECTED   : constant vertex_flags_t := 16#02#;
  GRAPH_HIDDEN     : constant vertex_flags_t := 16#04#;
  GRAPH_AUTOPLACED : constant vertex_flags_t := 16#08#;

  type vertex_t is record
    label_text  : vertex_label_t;
    label_su    : c.int;
    label_color : agar.core.types.uint32_t;
    bg_color    : agar.core.types.uint32_t;
    style       : vertex_style_t;
    flags       : vertex_flags_t;
    x           : c.int;
    y           : c.int;
    w           : c.unsigned;
    h           : c.unsigned;
    ptr         : agar.core.types.void_ptr_t;
    edges       : access edge_access_t;
    nedges      : c.unsigned;
    graph       : graph_access_t;
    vertices    : vertex_tail_queue.entry_t;
    sorted      : vertex_tail_queue.entry_t;
    popup_menu  : agar.gui.widget.menu.popup_menu_access_t;
  end record;
  pragma convention (c, vertex_t);

  subtype edge_flags_t is vertex_flags_t;

  type edge_t is record
    label_text  : vertex_label_t;
    label_su    : c.int;    
    edge_color  : agar.core.types.uint32_t;
    label_color : agar.core.types.uint32_t;
    flags       : edge_flags_t;
    v1          : vertex_access_t;
    v2          : vertex_access_t;
    ptr         : agar.core.types.void_ptr_t;
    graph       : graph_access_t;
    edges       : edge_tail_queue.entry_t;
    popup_menu  : agar.gui.widget.menu.popup_menu_access_t;
  end record;
  pragma convention (c, edge_t);

  subtype graph_flags_t is c.unsigned;
  GRAPH_HFILL     : constant graph_flags_t := 16#01#;
  GRAPH_VFILL     : constant graph_flags_t := 16#02#;
  GRAPH_EXPAND    : constant graph_flags_t := GRAPH_HFILL or GRAPH_VFILL;
  GRAPH_SCROLL    : constant graph_flags_t := 16#04#;
  GRAPH_DRAGGING  : constant graph_flags_t := 16#08#;
  GRAPH_PANNING   : constant graph_flags_t := 16#10#;
  GRAPH_NO_MOVE   : constant graph_flags_t := 16#20#;
  GRAPH_NO_SELECT : constant graph_flags_t := 16#40#;
  GRAPH_NO_MENUS  : constant graph_flags_t := 16#80#;
  GRAPH_READONLY  : constant graph_flags_t := GRAPH_NO_MOVE or GRAPH_NO_SELECT or GRAPH_NO_MENUS;

  type graph_t is record
    widget       : widget_t;
    flags        : graph_flags_t;
    width_pre    : c.int;
    height_pre   : c.int;
    x_offset     : c.int;
    y_offset     : c.int;
    x_min        : c.int;
    x_max        : c.int;
    y_min        : c.int;
    y_max        : c.int;
    horiz_bar    : agar.gui.widget.scrollbar.scrollbar_access_t;
    vertical_bar : agar.gui.widget.scrollbar.scrollbar_access_t;
    vertices     : vertex_tail_queue.head_t;
    edges        : edge_tail_queue.head_t;
    nvertices    : c.unsigned;
    nedges       : c.unsigned;
    px_min       : c.int;
    px_max       : c.int;
    py_min       : c.int;
    py_max       : c.int;
  end record;
  pragma convention (c, graph_t);

end agar.gui.widget.graph;