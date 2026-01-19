# Nested Enumerate

A simple [LaTeX](https://www.latex-project.org/) package with extended
levels of nesting than the default `enumerate` environment.

```latex
%%%%%%%%%%%%%% nested enumerate - nestenum %%%%%%%%%%
% 1.
% 2.
% 2.1.
% 2.2.
% 3. and so on....
% info in the companion on page 57 and classes.dtx on pages 33f.)
% all counters arabic
\newenvironment{nestenum}{\begin{enumerate}
\renewcommand{\theenumi}{\arabic{enumi}}
\renewcommand{\theenumii}{\arabic{enumii}}
\renewcommand{\theenumiii}{\arabic{enumiii}}
\renewcommand{\theenumiv}{\arabic{enumiv}}
% Label new defined
\renewcommand{\labelenumi}{\theenumi.}
\renewcommand{\labelenumii}{\theenumi.\theenumii.}
\renewcommand{\labelenumiii}{\theenumi.\theenumii.\theenumiii.}
\renewcommand{\labelenumiv}{\theenumi.\theenumii.\theenumiii.\theenumiv.}
% for the references
\renewcommand{\p@enumii}{\theenumi.}
\renewcommand{\p@enumiii}{\theenumi.\theenumii.}
\renewcommand{\p@enumiv}{\theenumi.\theenumii.\theenumiii.}}
{\end{enumerate}}
%%%%%%%%%%%%%% nestenum end %%%%%%%%%%%%%%%%%%%%%
```