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

#let tag-w = 30pt

#stack(
  dir: ttb,
  spacing: 12pt,

  stack(
    dir: ltr,
    mem-block(70pt, col-wall, "prol", subtext: "alloc: true"),

    mem-block(tag-w, col-header, "h"),
    mem-block(100pt, col-free, "free left"),
    mem-block(tag-w, col-header, "f"),

    mem-block(tag-w, col-header, "h"),
    mem-block(120pt, col-used, "target data", subtext: "freeing..."),
    mem-block(tag-w, col-header, "f"),

    mem-block(70pt, col-wall, "epil", subtext: "alloc: true"),
  ),

  grid(
    columns: (70pt + tag-w + 100pt, tag-w, tag-w, 120pt, tag-w, 70pt),
    [], align(center)[`^`], [], [], [], align(center)[`^`],
  ),
  grid(
    columns: (70pt + tag-w + 50pt, 180pt, 180pt),
    [], align(left)[`check left footer`], align(right)[`check right header`],
  ),
)
