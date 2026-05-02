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
  mem-block(80pt, col-free, "freed", subtext: "sz: 32 | ord: 5"),
  mem-block(110pt, col-free, "free buddy", subtext: "sz: 32 | order: 5"),
  mem-block(210pt, col-free, "free buddy", subtext: "sz: 64 | order: 6"),
)
