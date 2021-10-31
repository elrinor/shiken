#!/usr/bin/perl

if($#ARGV == -1) {
  print "gen - test sheet generator\n";
  print "USAGE:\n";
  print "  gen sheets-per-person <person-list \n";
  print "";
  print "Each line of person-list must have the following format:";
  print "  <person-code> <person-name>";
  print "For example:";
  print "  0920701 Duke the Awesome Guy";
  exit 0;
}

$pages = $ARGV[0];

print <<TEX;
\\documentclass{article}
\\usepackage[a4paper,vmargin={20mm,20mm},hmargin={20mm,20mm}]{geometry}
\\usepackage{graphicx}
\\usepackage[cp1251]{inputenc}
\\usepackage[russian]{babel}

\\setlength\\parindent{0mm}
\\setlength\\parskip{0mm} 

\\pagestyle{empty}

\\begin{document}
\\thispagestyle{empty}
TEX

while (<STDIN>) {
  if(/^([0-9]+)[ \t]+(.*?)[\n\r\t ]*$/i) {
    for($page = 1; $page <= $pages; $page++) {
      $code = $1 . $page;
      system "gen2of5 -s 0 -l 0 -i $code -o $code.png -w 2 -c";
      print <<TEX;
\\begin{minipage}{0.5\\textwidth}
  \\parbox[t][1.6cm]{0.5\\textwidth}{
    \\begin{tabular}{|r|l|}
      \\hline Имя & $2 \\\\
      \\hline Код & $code \\\\
      \\hline Страница & $page \\\\
      \\hline
    \\end{tabular}
  }
\\end{minipage}
\\begin{minipage}{0.5\\textwidth}
  \\begin{flushright}
    \\fbox{
      \\includegraphics[width=1.5cm,height=1.5cm]{anchor_l.png}
      \\includegraphics[width=4cm,height=1.5cm]{$code.png}
      \\includegraphics[width=1.5cm,height=1.5cm]{anchor_r.png}
    }
  \\end{flushright}
\\end{minipage}

Запишите решение строго \\textbf{внутри} рамки: \\\\
\\fbox{
  \\parbox[t][23.3cm]{16.6cm}{\\hfill}
}
\\newpage


TEX
    }
  }
}

print <<TEX;
\\end{document}
TEX










