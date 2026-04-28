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
    #if subtext != "" [ \ #text(fill: col-ptr, size: 10pt, subtext) ]
  ],
)

#let chunk-w = 125pt

#stack(
  dir: ttb,
  spacing: 8pt,

  stack(
    dir: ltr,
    mem-block(chunk-w, col-free, "new free", subtext: "next -> free 1"),
    mem-block(chunk-w, col-free, "free 1", subtext: "next -> free 2"),
    mem-block(chunk-w, col-used, "data 2"),
    mem-block(chunk-w, col-free, "free 2", subtext: "next -> null"),
  ),

  grid(
    columns: (chunk-w, chunk-w * 3),
    align(left)[`^ head & start_ptr`], [],
  ),
)
