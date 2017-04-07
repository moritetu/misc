Autoconf Sample
===============

## Generate Makefile

```
aclocal
autoconf
automake -ac
./configure
make
./sample_program
```

Or

```
sh autogen.sh
./configure
make
./sample_program
```

## Remove generated files

```
sh antigen.sh
```

