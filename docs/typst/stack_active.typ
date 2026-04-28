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

#let mem-block(w, bg, label) = rect(
  width: w,
  height: 45pt,
  fill: bg,
  stroke: 1pt + col-border,
  align(center + horizon)[#text(fill: white, weight: "bold", label)],
)

#let h1-w = 40pt
#let d1-w = 120pt
#let pad-w = 20pt
#let h2-w = 40pt
#let d2-w = 120pt
#let free-w = 160pt

#let used-w = h1-w + d1-w + pad-w + h2-w + d2-w

#stack(
  dir: ttb,
  spacing: 8pt,

  stack(
    dir: ltr,
    mem-block(h1-w, col-header, "h1"),
    mem-block(d1-w, col-used, "data 1"),
    mem-block(pad-w, col-pad, "p"),
    mem-block(h2-w, col-header, "h2"),
    mem-block(d2-w, col-used, "data 2"),
    mem-block(free-w, col-free, "free space"),
  ),

  grid(
    columns: (used-w, free-w),
    align(left)[`^ m_start_ptr`], align(left)[`^ m_offset`],
  ),
)
