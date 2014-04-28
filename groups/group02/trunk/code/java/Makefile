JCFLAGS=-d $(BUILDDIR) -sourcepath $(SOURCEDIR) -cp $(BUILDDIR)
JARFLAGS=cfe $(JARFILE) $(JARENTRY) -C $(BUILDDIR) .
JVMFLAGS=-jar $(JARFILE)
JC = javac
JVM = java
JAR = jar

BUILDDIR = build
SOURCEDIR = src
JARFILE = $(BUILDDIR)/DHT.jar
JARENTRY = dht.GUI


.SUFFIXES: .java .class
.java.class:
	$(JC) $< $(JCFLAGS)


SOURCES := $(wildcard $(SOURCEDIR)/**/*.java)
CLASSES := $(wildcard $(BUILDDIR)/**/*.class)

all: jar

compile: mkdir $(SOURCES:.java=.class)

jar: compile
	$(JAR) $(JARFLAGS) $(CLASSES)

run: jar
	/bin/chmod 744 $(JARFILE)
	$(JVM) $(JVMFLAGS) $(JARFILE)

mkdir:
	/bin/mkdir -p $(BUILDDIR)

clean:
	/bin/rm -r -f $(BUILDDIR)