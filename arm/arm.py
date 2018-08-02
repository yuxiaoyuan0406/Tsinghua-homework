#!/usr/bin/python
# -*- coding: UTF-8 -*-


class Point:
    '平面直角坐标系下点的基类'

    def __init__(self, x, y):
        self.x = x
        self.y = y

    def complexNum(self):
        print(complex(self.x, self.y))
        return complex(self.x, self.y)


A = Point(2.0, 1.0)
A.complexNum()
