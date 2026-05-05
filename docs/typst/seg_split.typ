#set page(width: auto, height: auto, margin: 16pt, fill: none)
#set text(
  font: "BigBlueTerm437 Nerd Font Mono",
  fill: rgb("#C9D1D9"),
  size: 14pt,
)

#let col-used = rgb("1F6FEB")
#let col-header = rgb("#238636")
#let col-free = rgb("#21262D")
#let col-border = rgb("#8B949E")
#let col-ptr = rgb("#A5D6FF")
#let mem-block(w, bg, label, subtext: "") = rect(
  width: w,
  height: 45pt,
  fill: bg,
  stroke: 1pt + col-border,
  align(center + horizon)[#text(fill: white, weight: "bold", label)#if (
      subtext != ""
    ) [ \ #text(fill: col-ptr, size: 7pt, subtext) ]],
)

#let tag-w = 30pt
#let block-w = 160pt

#stack(dir: ttb, spacing: 20pt, stack(
  dir: ltr,
  mem-block(tag-w, col-header, "h", subtext: "o: 7"),
  mem-block(
    block-w,
    col-free,
    "free 1",
    subtext: "left buddy | size: 128",
  ),
  mem-block(tag-w, col-header, "f", subtext: "o: 7"),

  mem-block(tag-w, col-header, "h", subtext: "o: 7"),
  mem-block(block-w, col-free, "free 2", subtext: "right buddy | size: 128"),
  mem-block(tag-w, col-header, "f", subtext: "o: 7"),
))
