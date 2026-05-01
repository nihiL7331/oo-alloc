#set page(width: auto, height: auto, margin: 16pt, fill: none)
#set text(
  font: "BigBlueTerm437 Nerd Font Mono",
  fill: rgb("#C9D1D9"),
  size: 14pt,
)

#let col-used = rgb("1F6FEB")
#let col-header = rgb("#238636")
#let col-pad = rgb("#DA3633")
#let col-free = rgb("#21262D")
#let col-border = rgb("#8B949E")
#let col-ptr = rgb("#A5D6FF")

#let mem-block(w, bg, label, subtext: "") = rect(
  width: w,
  height: 45pt,
  fill: bg,
  stroke: 1pt + col-border,
  align(center + horizon)[
    #text(fill: white, weight: "bold", label)
    #if subtext != "" [ \ #text(fill: col-ptr, size: 7pt, subtext) ]
  ],
)

#stack(
  dir: ltr,
  spacing: 12pt,

  stack(
    dir: ttb,
    spacing: 16pt,
    stack(
      dir: ltr,
      mem-block(60pt, col-pad, "pad", subtext: "back_ptr"),
      mem-block(30pt, col-header, "h", subtext: "size"),
      mem-block(130pt, col-used, "data", subtext: "aligned ->"),
      mem-block(30pt, col-header, "f", subtext: "size"),
    ),
  ),

  align(horizon)[#line(length: 45pt, angle: 90deg, stroke: 1pt + col-border)],

  stack(
    dir: ttb,
    spacing: 16pt,
    stack(
      dir: ltr,
      mem-block(30pt, col-header, "h", subtext: "size"),
      mem-block(
        150pt,
        col-free,
        "tree node",
        subtext: "parent, left, right, red",
      ),
      mem-block(40pt, col-free, "... ", subtext: "empty"),
      mem-block(30pt, col-header, "f", subtext: "size"),
    ),
  ),
)
