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

#let used-width = 280pt
#let free-width = 220pt

#stack(
  dir: ttb,
  spacing: 8pt,
  stack(
    dir: ltr,
    mem-block(120pt, col-used, "alloc 1"),
    mem-block(20pt, col-pad, ""),
    mem-block(140pt, col-used, "alloc 2"),
    mem-block(free-width, col-free, "free space"),
  ),
  grid(
    columns: (used-width, free-width),
    align(left)[`^ m_start_ptr`], align(left)[`^ m_offset`],
  ),
)
