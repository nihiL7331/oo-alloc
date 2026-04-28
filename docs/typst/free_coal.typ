#set page(
  width: auto,
  height: auto,
  margin: 16pt,
  fill: none,
)

#set text(
  font: "BigBlueTerm437 Nerd Font Mono",
  fill: rgb("#C9D1D9"),
  size: 14pt,
)
#let col-used = rgb("1F6FEB");
#let col-header = rgb("#238636")
#let col-pad = rgb("#DA3633");
#let col-free = rgb("#21262D");
#let col-border = rgb("#8B949E");
#let col-ptr = rgb("#A5D6FF")

#let mem-block(w, bg, label, subtext: "") = rect(
  width: w,
  height: 45pt,
  fill: bg,
  stroke: 1pt + col-border,
  align(center + horizon)[
    #text(fill: white, weight: "bold", label)
    #if subtext != "" [ \ #text(fill: col-ptr, size: 6.9pt, subtext) ]
  ],
)

#let h_new = 30pt
#let d_new = 20pt
#let f1-w = 70pt
#let h1-w = 30pt
#let d1-w = 100pt

#let merged-w = 250pt


#stack(
  dir: ttb,
  spacing: 8pt,

  stack(
    dir: ltr,
    mem-block(h_new, col-header, "h"),
    mem-block(d_new, col-used, "d3"),
    mem-block(f1-w, col-free, "free 1", subtext: "sz:70|nxt:f2"),
    mem-block(h1-w, col-header, "h"),
    mem-block(d1-w, col-used, "data 1"),
    mem-block(
      merged-w,
      col-free,
      "free 2 (merged)",
      subtext: "size: 250 | next: null",
    ),
  ),

  grid(
    columns: (h_new + d_new, 500pt - (h_new + d_new)),
    [], align(left)[`^ m_free_list_head`],
  ),
)
