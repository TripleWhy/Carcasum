TEMPLATE = subdirs
SUBDIRS = Carcasum

core {
} else:tournament {
} else {
SUBDIRS += quazip
Carcasum.depends = quazip
}
