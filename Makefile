# Makefile pour un unique exécutable

# liste des variables à renseigner
#   CIBLE : nom du programme ( $(CIBLE).c doit contenir main() )
#   SRCS : ensemble des fichiers sources 
#   LIBS : liste des bibliothèques utiles à l'édition des liens 
#          (format : -lnom1 -lnom2 ...) 
#   PREFIX : chemin de la hiérarchie 
#
# NE PAS OUBLIER D'AJOUTER LA LISTE DES DEPENDANCES A LA FIN DU FICHIER

CIBLE = main
SRCS =  src/Camera.cpp main.cpp src/Trackball.cpp src/imageLoader.cpp src/Mesh.cpp 
LIBS =  -lglut -lGLU -lGL -lm -lpthread 
#########################################################"

BUILDDIR ?= build
OBJDIR ?= $(BUILDDIR)/obj
BINDIR ?= $(BUILDDIR)
PREFIX ?= $(HOME)/.local
INSTALLBIN ?= $(PREFIX)/bin

INCDIR = .
LIBDIR = .

# nom du compilateur
CC = g++
CPP = g++

# options du compilateur          
CFLAGS = -Wall -O3 
CXXFLAGS = -Wall -O3 

# option du preprocesseur
CPPFLAGS =  -I$(INCDIR) 

# options du linker et liste des bibliothèques à charger
LDFLAGS = -L/usr/X11R6/lib              
LDLIBS = -L$(LIBDIR) $(LIBS)  

# construire la liste des fichiers objets une nouvelle chaine à partir
# de SRCS en substituant les occurences de ".c" par ".o" 
OBJS = $(SRCS:%.cpp=$(OBJDIR)/%.o)

.PHONY: all clean veryclean install installdirs dep

all: $(BINDIR)/$(CIBLE)

$(BINDIR)/$(CIBLE): $(OBJS) | $(BINDIR)
	$(CPP) $(CXXFLAGS) -o $@ $(OBJS) $(LDFLAGS) $(LDLIBS)

# règle pour compiler les fichiers .cpp en .o
$(OBJDIR)/%.o: %.cpp | $(OBJDIR)
	mkdir -p $(dir $@)
	$(CPP) $(CXXFLAGS) $(CPPFLAGS) -c $< -o $@

$(BINDIR):
	mkdir -p $@

$(OBJDIR):
	mkdir -p $@

install: $(BINDIR)/$(CIBLE)
	install -d $(INSTALLBIN)
	install $(BINDIR)/$(CIBLE) $(INSTALLBIN)/

installdirs:
	test -d $(INCDIR) || mkdir $(INCDIR)
	test -d $(LIBDIR) || mkdir $(LIBDIR)
	test -d $(BINDIR) || mkdir $(BINDIR)

clean:
	rm -rf $(BUILDDIR) *~ $(CIBLE) $(OBJS)

veryclean: clean
	rm -f $(BINDIR)/$(CIBLE)

dep:
	gcc $(CPPFLAGS) -MM $(SRCS)

# liste des dépendances générée par 'make dep'
$(OBJDIR)/src/Camera.o: src/Camera.cpp src/Camera.h src/Vec3.h src/Trackball.h
$(OBJDIR)/main.o: main.cpp src/Camera.h src/Vec3.h src/Trackball.h src/Scene.h \
 src/Mesh.h src/Material.h src/imageLoader.h src/Ray.h src/Line.h \
 src/Triangle.h src/Plane.h src/Sphere.h src/Square.h src/matrixUtilities.h
$(OBJDIR)/src/Trackball.o: src/Trackball.cpp src/Trackball.h
$(OBJDIR)/src/imageLoader.o: src/imageLoader.cpp src/imageLoader.h src/Vec3.h
$(OBJDIR)/src/Mesh.o: src/Mesh.cpp src/Mesh.h src/Material.h src/Vec3.h \
 src/imageLoader.h src/Ray.h src/Line.h src/Triangle.h src/Plane.h


