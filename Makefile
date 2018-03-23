PY_DIRS := tests

PYTHON_BIN ?= pipenv run python
NOSETESTS_BIN ?= pipenv run nosetests
FLAKE8_BIN ?= pipenv run flake8

all: install test stylecheck

build:
	$(PYTHON_BIN) setup.py build

install:
	$(PYTHON_BIN) setup.py install

test:
	$(NOSETESTS_BIN) tests

stylecheck:
	$(FLAKE8_BIN) '--exclude=.svn,CVS,.bzr,.hg,.git,__pycache__,.tox,.#*' $(PY_DIRS)

clean:
	find $(PY_DIRS) -type d -name __pycache__ | xargs rm -rf
	rm -rf build

dist:
	@${PYTHON_BIN} setup.py sdist

.PHONY: all test stylecheck clean dist
