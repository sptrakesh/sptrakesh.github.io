# OpenAPI to LaTeX

I developed this as a testbed while familiarising myself with 
[Julia](https://julialang.org/).  A couple of companies I assisted also had a need for 
attaching API specifications with their customer contracts. The result of these 
needs is this project.

**Note:** I have since enhanced and rewritten the process in C++.  The old julia
implementation is still available in the [julia](https://github.com/sptrakesh/openapi2latex/tree/julia) branch.

The primary rationale for this utility is to be able to generate a PDF document that can 
be distributed to interested parties, when the source specifications are protected by access 
control (and where the said parties do not need to be provided with the access credentials).
I have used the tool to create specification documents that have been included in
customer contracts for a few companies that I have worked with.

The workflow is to use this process to generate the target LaTeX file, and run 
`xelatex` a few times (usually two times to get cross-references resolved) for 
the output PDF document. Also run `makeindex` to generate the document index if
desired.

```shell
<path to>/oa2tex -i <path to>/openapi.yaml \
  -o <path to output directory> \
  -s -c --use-cmark
cd <path to output directory>
xelatex -interaction=nonstopmode openapi
makeindex openapi
xelatex -interaction=nonstopmode openapi
```

## Structure
The generated LaTeX file has the following structure (you can of course modify the output 
file as desired):

* **Preamble** - Preamble for the document (`<path to output>/preamble.tex`).  Sets up various packages that are used.
  Edit the file as desired, especially the *main font* for the document.  The generator sets the main font to
  *Helvetica Neue* (`\setmainfont[Ligatures=TeX,Numbers=OldStyle]{Helvetica Neue}`).  Change to any system
  supported font as desired (through the command line option or by editing file as desired after generating the
  latex sources).
  * Sans-serif fonts like *Helvetica*, *Verdana*, *Calibri* ... are good for digital display of the PDF.
  * Use traditional print friendly serif fonts like the LaTeX default *Computer Modern* (comment out
    the `\setmainfont` directive), *Times New Roman*, Garamond ... if the primary purpose of 
    the output PDF is print.
  * You can also use modern variants of serif fonts such as *Merriweather*, *Sabon* ... which were 
    designed for both print and digital display.
  * See [examples](#petstore-samples) of the Petstore document with different fonts.
* **Frontmatter** - Titlepage and table of contents.
* **Mainmatter** - Contains two or three parts.
  * **Info** - The *info* object is presented as the first chapter.
  * **Examples** - Any examples that are defined in the `openapi.components.examples` section.
  * **Parameters** - Any parameters that are *referenced* from the various API endpoints. Will only list referenced
    parameters, not those that are defined *in-line*.
  * **Endpoints** - Part with the path operations grouped by tags. Each tag is presented in 
    a *chapter*.
  * **Schemas** - Part with the *schemas* declared and referenced in the specification. 
    Each *schema* is presented in a *chapter*. Schemas are listed alphabetically by their 
    filename and entity name.
  * **Responses** - Optional part with responses declared in the `openapi.components` structure. If 
    none are defined no part is created.
  * **Request Bodies** - Part with request bodies that are referenced, not if they are defined inline with schema references.
  * **Code Samples** - If the `x-codeSamples` extension exists for operations, these are 
    collected together into another part. Code samples are grouped together under each 
    tag group, which is presented as a *chapter*.
* **Backmatter**
  * List of *tables*.
  * **Index** - Operation ids and schema property names are added to index.  Note than
    `makeindex` must be run to generate the index.

See [openapi.pdf](https://github.com/sptrakesh/openapi2latex/blob/master/openapi.pdf)
for the PDF generated from the official [petstore](https://github.com/SLdragon/example-openapi-spec/blob/main/petstore-official.yaml)
specifications. Note that the petstore sample has a commonmark table in the information,
which the embedded converter does not support. This sample was generated using the 
`cmark` option.

## Usage
The generator is written in C++ and has a dependency on the [Boost](https://boost.org/) libraries.

Pre-built binaries for Mac OS and Windows are available on the [releases](https://github.com/sptrakesh/openapi2latex/releases) page.

**Note:** The old [Julia](https://julialang.org/) version of the utility is still available in the `julia` branch.

### Build project
To build the project install Boost and then checkout and build the project.

#### Install Dependencies

<tabs id="openapi-latex-build">
  <tab title="Mac OS" id="build-boost-macosx">
    <include from="boost.topic" element-id="boost-macosx"/>
  </tab>
  <tab title="Windows" id="build-boost-windows">
    <include from="boost.topic" element-id="boost-windows"/>
  </tab>
</tabs>

```shell
git clone https://github.com/sptrakesh/openapi-latex.git
cd openapi-latex
cmake -DCMAKE_PREFIX_PATH=/usr/local/boost \
  -DCMAKE_BUILD_TYPE=Release \
  -DCMAKE_INSTALL_PREFIX=/usr/local/spt \
  -S . -B build
cmake --build build -j12
sudo cmake --install build
```

### Command Line Options
The following options are supported by the 
[main.cpp](https://github.com/sptrakesh/openapi2latex/blob/master/src/main.cpp) executable:

* `--input | -i` - **Required**. The main OpenAPI specification file to parse.
* `--output | -o` - **Required**. The output LaTeX file to generate. Best to place 
  this at another location than the api specifications. Program will attempt to create 
  the directory tree if it does not exist. Since a lot of files are generated by this 
  process, and further when running LaTeX, it would be best to write the output to 
  a dedicated temporary directory.
* `--author | -a` - The author credit to show on the titlepage.
* `--footer | -f` - The right side footer text to display for the document.
* `--operation-summary | -s` - A flag to indicate that **Operation** summary should be used 
  as section headings instead of *operationId*.
* `--use-cmark | -m` - Use [cmark](https://github.com/commonmark/cmark) to convert `info.description`
  to latex. Recommended option, since the description can be quite long and complicated,
  and `cmark` should have much more comprehensive support for converting commonmark to 
  latex. Note the program uses the `cmark` utility via `std::system`, and not the library.
* `--log-level | -l` - Set the logging level (`critical|warn|info|debug`). Default `info`.
* `--console | -c` - Flag to indicate logs should also be echoed to `stdout`.
* `--log-dir | -z` - The directory under which the process log output is written.
  The directory *must* exist. Default `/tmp/` - note the mandatory trailing `/`.

#### Example
The following options were used to generate the sample petstore specifications document.
```shell
/usr/local/spt/bin/oa2tex \
  -i /tmp/petstore-official.yaml \
  -o /tmp/petstore \
  -s -c --use-cmark
```

#### Docker
A docker image is available for those who do not wish to build/install the utility on their computer.
Use docker mounts to hold the source specification files as well as the destination output files. Once
the output LaTeX files are generated, run `xelatex` as appropriate.

**Note:** `cmark` *is* installed in the docker image.

A sample invocation is as follows:
```shell
docker run --rm -it \
  -v $PWD/tmp/test:/tmp/test \
  sptrakesh/openapi-latex \
  -i /tmp/test/docs/openapi.yaml \
  -o /tmp/test/specs \
  -s -m -c
```

**Note:** The output `openapi.tex` includes several other `.tex` files generated by the
process with an `\input` directive.  All file paths are absolute paths, hence the docker
mount will need to set up appropriately, or the output moved to an appropriate location
for the paths to work on the host machine.

##### Wrap Image
If you do not wish to have [texlive](https://tug.org/texlive/) installed on your local
machine, you can create a wrapper image which also provides `xelatex` and run the
entire process in a container.

```Docker
FROM sptrakesh/openapi-latex
USER root
RUN apk add --no-cache texlive-xetex
USER spt
ENTRYPOINT [ "/bin/sh" ]
```

## Extensions
A few extensions to the specifications developed by [Redocly](https://redocly.com/) are supported.

* Source code samples are parsed from the `x-codeSamples` array attached to an operation. 
  All code samples are attached to a separate part of the output document, and follow the 
  same chapter organisation as the API tags.
* Tag groups are parsed from the `x-tagGroups` array attached to the root of the specification 
  document. If specified, an initial chapter **Tag Groups** is added, which lists the groupings 
  with links to the Tag chapters.
* An additional custom extension for documenting the *version* from which an API operation
  or schema or property has been added is supported in the form of `x-since-version`.  The
  generated LaTeX files will include this information if so annotated in the API specifications.
  Example: `x-since-version: 2.6.8`
  * Document for **Operations** and **Schemas**.  See sample screen captures of operation
    version and schema version as rendered in a PDF

<tabs id="extension-samples">
  <tab title="Operation" id="extenstion-operation">
    <img src="operation-since-version.png" alt="Operation Version" thumbnail="false"/>
  </tab>
  <tab title="Schema" id="extenstion-schema">
    <img src="schema-since-version.png" alt="Operation Version" thumbnail="false"/>
  </tab>
</tabs>

## Examples

Screen captures of a TOC page from the petstore specification document with
different fonts.

<tabs id="petstore-samples">
  <tab title="Helvetica Neue" id="petstore-samples-helvetica">
    Document generated using *Helvetica Neue* font.
    <img src="petstore-helvetica.png" alt="Helvetica Neue" thumbnail="true"/>
  </tab>
  <tab title="Calibri" id="petstore-samples-calibri">
    Document generated using *Calibri* font.
    <img src="petstore-calibri.png" alt="Calibri" thumbnail="true"/>
  </tab>
  <tab title="Times New Roman" id="petstore-samples-times">
    Document generated using *Times New Roman* font.
    <img src="petstore-times.png" alt="Times New Roman" thumbnail="true"/>
  </tab>
  <tab title="Computer Modern Roman" id="petstore-samples-cmr">
    Document generated using the default *Computer Modern Roman* font.
    <img src="petstore-cmr.png" alt="Computer Modern Roman" thumbnail="true"/>
  </tab>
  <tab title="Merriweather" id="petstore-samples-merriweather">
    Document generated using the classic *Merriweather* font.
    <img src="petstore-merriweather.png" alt="Merriweather" thumbnail="true"/>
  </tab>
  <tab title="Merriweather Light" id="petstore-samples-merriweather-light">
    Document generated using *Merriweather Light* font.
    <img src="petstore-merriweather-light.png" alt="Merriweather Light" thumbnail="true"/>
  </tab>
  <tab title="Sabon" id="petstore-samples-sabon">
    Document generated using classic *Sabon* font.
    <img src="petstore-sabon.png" alt="Sabon" thumbnail="true"/>
  </tab>
  <tab title="Sabon eText" id="petstore-samples-sabon-etext">
    Document generated using modern *Sabon eText* font.
    <img src="petstore-sabon-etext.png" alt="Sabon eText" thumbnail="true"/>
  </tab>
</tabs>

## Limitations
Probably too many to list, but the following items should be kept in mind.

* This utility is based on the way *I write API specifications*, and markup descriptions.
* Schema objects are assumed to model closely their organisation in a source code implementation.
  This in turn implies that nested structures are represented as schema references, and not
  listed in-line in the schema. Deeply nested in-line schemas would be very hard to represent
  in a printed document in any case.
* Mainly tested with specifications that are split into individual files - representing 
  paths, schemas, parameters etc. Most testing has been against large handwritten specifications,
  which follows the principles laid out in [split specifications](https://davidgarcia.dev/posts/how-to-split-open-api-spec-into-multiple-files/).
  A few simple single file specifications have also been tested.
* Mainly supports OpenAPI specification version [3.0.3](https://spec.openapis.org/oas/v3.0.3),
  although some properties from [3.1.0](https://spec.openapis.org/oas/latest.html) are also 
  included.
* Only supports loading local specification files in **YAML** format. **JSON** is not 
  supported at present.
* Not all properties/aspects of the specification are output in the generated LaTeX file. 
  I selected what I felt are most relevant to be shared.
* Markdown markup may not be fully translated to LaTeX. See [convert.cpp](https://github.com/sptrakesh/openapi2latex/blob/master/test/convert.cpp)
  for basic rules implemented.
  * Bold/italic blocks of text (spanning paragraphs) are not supported.

The output is a wrapping LaTeX file that includes several smaller included files,
and hence can be easily modified as needed to further customise the final PDF document.

## Dependencies
* **[rapidyaml](https://github.com/biojppm/rapidyaml)** YAML parser library. Via `cmake fetchcontent`
* **[Boost](https://boost.org/)** local installation required.
* **[Clara](https://github.com/catchorg/Clara)** - Command line options parser. Included in project.
* **[Catch2](https://github.com/catchorg/Catch2)** - Testing framework. Via `cmake fetchcontent`
* **[NanoLog](https://github.com/Iyengar111/NanoLog)** - Logging framework used for the project. Modified version included in project.
* **[nestenum](Nested-Enumerate.md)** Simple LaTeX package for nested enumerations.
  * Add to your local texmf tree.  For instance, on Mac OS, this is stored in `~/Library/texmf/tex/latex/nestenum.sty`.