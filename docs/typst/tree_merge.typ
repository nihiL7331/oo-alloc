#set page(width: auto, height: auto, margin: 16pt, fill: none)
#set text(
  font: "BigBlueTerm437 Nerd Font Mono",
  fill: rgb("#C9D1D9"),
  size: 14pt,
)

#let col-used = rgb("1F6FEB")
#let col-header = rgb("#238636")
#let col-wall = rgb("#D29922")
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
  dir: ttb,
  spacing: 12pt,

  stack(
    dir: ltr,
    mem-block(70pt, col-wall, "prol", subtext: "alloc: true"),

    mem-block(30pt, col-header, "h"),
    mem-block(
      280pt,
      col-free,
      "merged free block",
      subtext: "size: 340 | inserted to tree",
    ),
    mem-block(30pt, col-header, "f"),

    mem-block(70pt, col-wall, "epil", subtext: "alloc: true"),
  ),
)
