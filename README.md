## <b>ppm</b> is a pretty printer for mathematics

![alt text](doc/img/sin_blended.png)

### Usage

ppm <b>input</b>

Like:

`ppm "x + 3"`

### Fonts and Colors

ppm supports a number of different fonts (see the full list with ppm --help). Color can either be alternating (the default), or grouped. The `--no-random` flag disables random colors.

Example:

`ppm --font ivrit --color grouped "tan(y) / x"`

![alt text](doc/img/tan_smaller.png)

### Building

Run `build.sh` to create the <b>ppm</b> executable.<br>
Then run `examples.sh` to see <b>ppm</b> in action.

### Optional Installation

cd project<br>
(sudo) make install
