// 1. Pobieramy nazwę grupy z CLI (domyślnie 'FragSearch' do testów)
#let target-group = sys.inputs.at("group", default: "frag_search")

#set page(
  width: 640pt,
  height: auto,
  fill: rgb("21262D"),
  margin: 30pt,
  background: pad(
    1pt,
    rect(
      width: 100%,
      height: 100%,
      stroke: 8pt + rgb("8B949E"),
    ),
  ),
)

// 2. Twoja paleta GitHub Dark Retro
#let col-blue = rgb("1F6FEB")
#let col-green = rgb("238636")
#let col-red = rgb("DA3633")
#let col-bg = rgb("21262D")
#let col-border = rgb("8B949E")
#let col-lightblue = rgb("A5D6FF")
#let col-purple = rgb("8957E5")

#set text(
  font: "BigBlueTerm437 Nerd Font Mono",
  fill: col-lightblue,
  size: 14pt,
)

// 3. Wczytanie i odfiltrowanie TYLKO konkretnej grupy
#let raw-data = json("results.json")
#let benchmarks = raw-data.benchmarks.filter(b => (
  b.at("run_type", default: "iteration") == "iteration"
    and "cpu_time" in b
    and b.name.starts-with(target-group)
))

// Jeśli min-time jest mniejszy niż 1, ustawiamy na 1, żeby logarytm nie zwariował
#let min-time = calc.max(1.0, calc.min(..benchmarks.map(b => b.cpu_time)))
#let max-time = calc.max(..benchmarks.map(b => b.cpu_time))
#let max-bar-width = 300pt

#text(
  fill: white,
  size: 18pt,
)[= #target-group (ns - log scale)]
#v(20pt)

#grid(
  columns: (auto, 1fr),
  row-gutter: 20pt,
  column-gutter: 20pt,
  align: horizon,

  ..for b in benchmarks {
    let log-time = calc.log(calc.max(1.0, b.cpu_time), base: 10)
    let log-max = calc.log(max-time, base: 10)
    let current-width = (log-time / log-max) * max-bar-width

    let clean-name = b.name.replace(target-group + "/", "")

    let bar-color = if (
      clean-name.starts-with("arena")
        or clean-name.starts-with("stack")
        or clean-name.starts-with("pool")
    ) {
      col-green
    } else if (
      clean-name.starts-with("free") or clean-name.starts-with("segregated")
    ) {
      col-blue
    } else if (
      clean-name.starts-with("buddy") or clean-name.starts-with("slab")
    ) {
      col-purple
    } else if clean-name.starts-with("malloc") {
      col-red
    } else {
      col-red
    }

    (
      [*#clean-name*],
      stack(
        dir: ltr,
        spacing: 10pt,
        rect(width: current-width, height: 16pt, fill: bar-color),
        align(horizon)[#str(calc.round(b.cpu_time, digits: 1)) ns],
      ),
    )
  }
)
