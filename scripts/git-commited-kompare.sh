#!/bin/bash
git diff HEAD^1 $* | kompare -
