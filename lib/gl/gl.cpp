// RasterIX
// https://github.com/ToNi3141/RasterIX
// Copyright (c) 2024 ToNi3141

// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.

// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <https://www.gnu.org/licenses/>.

#define NOMINMAX // Windows workaround
#include "gl.h"
#include "opengl/glImpl.h"

// Wrapper
// Open GL 1.0
// -------------------------------------------------------
GLAPI_WRAPPER void APIENTRY glAccum(GLenum op, GLfloat value) { impl_glAccum(op, value); }
GLAPI_WRAPPER void APIENTRY glAlphaFunc(GLenum func, GLclampf ref) { impl_glAlphaFunc(func, ref); }
GLAPI_WRAPPER void APIENTRY glBegin(GLenum mode) { impl_glBegin(mode); }
GLAPI_WRAPPER void APIENTRY glBitmap(GLsizei width, GLsizei height, GLfloat xOrig, GLfloat yOrig, GLfloat xMove, GLfloat yMove, const GLubyte* bitmap) { impl_glBitmap(width, height, xOrig, yOrig, xMove, yMove, bitmap); }
GLAPI_WRAPPER void APIENTRY glBlendFunc(GLenum srcFactor, GLenum dstFactor) { impl_glBlendFunc(srcFactor, dstFactor); }
GLAPI_WRAPPER void APIENTRY glCallList(GLuint list) { impl_glCallList(list); }
GLAPI_WRAPPER void APIENTRY glCallLists(GLsizei n, GLenum type, const GLvoid* lists) { impl_glCallLists(n, type, lists); }
GLAPI_WRAPPER void APIENTRY glClear(GLbitfield mask) { impl_glClear(mask); }
GLAPI_WRAPPER void APIENTRY glClearAccum(GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha) { impl_glClearAccum(red, green, blue, alpha); }
GLAPI_WRAPPER void APIENTRY glClearColor(GLclampf red, GLclampf green, GLclampf blue, GLclampf alpha) { impl_glClearColor(red, green, blue, alpha); }
GLAPI_WRAPPER void APIENTRY glClearDepth(GLclampd depth) { impl_glClearDepth(depth); }
GLAPI_WRAPPER void APIENTRY glClearIndex(GLfloat c) { impl_glClearIndex(c); }
GLAPI_WRAPPER void APIENTRY glClearStencil(GLint s) { impl_glClearStencil(s); }
GLAPI_WRAPPER void APIENTRY glClipPlane(GLenum plane, const GLdouble* equation) { impl_glClipPlane(plane, equation); }
GLAPI_WRAPPER void APIENTRY glColor3b(GLbyte red, GLbyte green, GLbyte blue) { impl_glColor3b(red, green, blue); }
GLAPI_WRAPPER void APIENTRY glColor3bv(const GLbyte* v) { impl_glColor3bv(v); }
GLAPI_WRAPPER void APIENTRY glColor3d(GLdouble red, GLdouble green, GLdouble blue) { impl_glColor3d(red, green, blue); }
GLAPI_WRAPPER void APIENTRY glColor3dv(const GLdouble* v) { impl_glColor3dv(v); }
GLAPI_WRAPPER void APIENTRY glColor3f(GLfloat red, GLfloat green, GLfloat blue) { impl_glColor3f(red, green, blue); }
GLAPI_WRAPPER void APIENTRY glColor3fv(const GLfloat* v) { impl_glColor3fv(v); }
GLAPI_WRAPPER void APIENTRY glColor3i(GLint red, GLint green, GLint blue) { impl_glColor3i(red, green, blue); }
GLAPI_WRAPPER void APIENTRY glColor3iv(const GLint* v) { impl_glColor3iv(v); }
GLAPI_WRAPPER void APIENTRY glColor3s(GLshort red, GLshort green, GLshort blue) { impl_glColor3s(red, green, blue); }
GLAPI_WRAPPER void APIENTRY glColor3sv(const GLshort* v) { impl_glColor3sv(v); }
GLAPI_WRAPPER void APIENTRY glColor3ub(GLubyte red, GLubyte green, GLubyte blue) { impl_glColor3ub(red, green, blue); }
GLAPI_WRAPPER void APIENTRY glColor3ubv(const GLubyte* v) { impl_glColor3ubv(v); }
GLAPI_WRAPPER void APIENTRY glColor3ui(GLuint red, GLuint green, GLuint blue) { impl_glColor3ui(red, green, blue); }
GLAPI_WRAPPER void APIENTRY glColor3uiv(const GLuint* v) { impl_glColor3uiv(v); }
GLAPI_WRAPPER void APIENTRY glColor3us(GLushort red, GLushort green, GLushort blue) { impl_glColor3us(red, green, blue); }
GLAPI_WRAPPER void APIENTRY glColor3usv(const GLushort* v) { impl_glColor3usv(v); }
GLAPI_WRAPPER void APIENTRY glColor4b(GLbyte red, GLbyte green, GLbyte blue, GLbyte alpha) { impl_glColor4b(red, green, blue, alpha); }
GLAPI_WRAPPER void APIENTRY glColor4bv(const GLbyte* v) { impl_glColor4bv(v); }
GLAPI_WRAPPER void APIENTRY glColor4d(GLdouble red, GLdouble green, GLdouble blue, GLdouble alpha) { impl_glColor4d(red, green, blue, alpha); }
GLAPI_WRAPPER void APIENTRY glColor4dv(const GLdouble* v) { impl_glColor4dv(v); }
GLAPI_WRAPPER void APIENTRY glColor4f(GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha) { impl_glColor4f(red, green, blue, alpha); }
GLAPI_WRAPPER void APIENTRY glColor4fv(const GLfloat* v) { impl_glColor4fv(v); }
GLAPI_WRAPPER void APIENTRY glColor4i(GLint red, GLint green, GLint blue, GLint alpha) { impl_glColor4i(red, green, blue, alpha); }
GLAPI_WRAPPER void APIENTRY glColor4iv(const GLint* v) { impl_glColor4iv(v); }
GLAPI_WRAPPER void APIENTRY glColor4s(GLshort red, GLshort green, GLshort blue, GLshort alpha) { impl_glColor4s(red, green, blue, alpha); }
GLAPI_WRAPPER void APIENTRY glColor4sv(const GLshort* v) { impl_glColor4sv(v); }
GLAPI_WRAPPER void APIENTRY glColor4ub(GLubyte red, GLubyte green, GLubyte blue, GLubyte alpha) { impl_glColor4ub(red, green, blue, alpha); }
GLAPI_WRAPPER void APIENTRY glColor4ubv(const GLubyte* v) { impl_glColor4ubv(v); }
GLAPI_WRAPPER void APIENTRY glColor4ui(GLuint red, GLuint green, GLuint blue, GLuint alpha) { impl_glColor4ui(red, green, blue, alpha); }
GLAPI_WRAPPER void APIENTRY glColor4uiv(const GLuint* v) { impl_glColor4uiv(v); }
GLAPI_WRAPPER void APIENTRY glColor4us(GLushort red, GLushort green, GLushort blue, GLushort alpha) { impl_glColor4us(red, green, blue, alpha); }
GLAPI_WRAPPER void APIENTRY glColor4usv(const GLushort* v) { impl_glColor4usv(v); }
GLAPI_WRAPPER void APIENTRY glColorMask(GLboolean red, GLboolean green, GLboolean blue, GLboolean alpha) { impl_glColorMask(red, green, blue, alpha); }
GLAPI_WRAPPER void APIENTRY glColorMaterial(GLenum face, GLenum mode) { impl_glColorMaterial(face, mode); }
GLAPI_WRAPPER void APIENTRY glCopyPixels(GLint x, GLint y, GLsizei width, GLsizei height, GLenum type) { impl_glCopyPixels(x, y, width, height, type); }
GLAPI_WRAPPER void APIENTRY glCullFace(GLenum mode) { impl_glCullFace(mode); }
GLAPI_WRAPPER void APIENTRY glDeleteLists(GLuint list, GLsizei range) { impl_glDeleteLists(list, range); }
GLAPI_WRAPPER void APIENTRY glDepthFunc(GLenum func) { impl_glDepthFunc(func); }
GLAPI_WRAPPER void APIENTRY glDepthMask(GLboolean flag) { impl_glDepthMask(flag); }
GLAPI_WRAPPER void APIENTRY glDepthRange(GLclampd zNear, GLclampd zFar) { impl_glDepthRange(zNear, zFar); }
GLAPI_WRAPPER void APIENTRY glDisable(GLenum cap) { impl_glDisable(cap); }
GLAPI_WRAPPER void APIENTRY glDrawBuffer(GLenum mode) { impl_glDrawBuffer(mode); }
GLAPI_WRAPPER void APIENTRY glDrawPixels(GLsizei width, GLsizei height, GLenum format, GLenum type, const GLvoid* pixels) { impl_glDrawPixels(width, height, format, type, pixels); }
GLAPI_WRAPPER void APIENTRY glEdgeFlag(GLboolean flag) { impl_glEdgeFlag(flag); }
GLAPI_WRAPPER void APIENTRY glEdgeFlagv(const GLboolean* flag) { impl_glEdgeFlagv(flag); }
GLAPI_WRAPPER void APIENTRY glEnable(GLenum cap) { impl_glEnable(cap); }
GLAPI_WRAPPER void APIENTRY glEnd(void) { impl_glEnd(); }
GLAPI_WRAPPER void APIENTRY glEndList(void) { impl_glEndList(); }
GLAPI_WRAPPER void APIENTRY glEvalCoord1d(GLdouble u) { impl_glEvalCoord1d(u); }
GLAPI_WRAPPER void APIENTRY glEvalCoord1dv(const GLdouble* u) { impl_glEvalCoord1dv(u); }
GLAPI_WRAPPER void APIENTRY glEvalCoord1f(GLfloat u) { impl_glEvalCoord1f(u); }
GLAPI_WRAPPER void APIENTRY glEvalCoord1fv(const GLfloat* u) { impl_glEvalCoord1fv(u); }
GLAPI_WRAPPER void APIENTRY glEvalCoord2d(GLdouble u, GLdouble v) { impl_glEvalCoord2d(u, v); }
GLAPI_WRAPPER void APIENTRY glEvalCoord2dv(const GLdouble* u) { impl_glEvalCoord2dv(u); }
GLAPI_WRAPPER void APIENTRY glEvalCoord2f(GLfloat u, GLfloat v) { impl_glEvalCoord2f(u, v); }
GLAPI_WRAPPER void APIENTRY glEvalCoord2fv(const GLfloat* u) { impl_glEvalCoord2fv(u); }
GLAPI_WRAPPER void APIENTRY glEvalMesh1(GLenum mode, GLint i1, GLint i2) { impl_glEvalMesh1(mode, i1, i2); }
GLAPI_WRAPPER void APIENTRY glEvalMesh2(GLenum mode, GLint i1, GLint i2, GLint j1, GLint j2) { impl_glEvalMesh2(mode, i1, i2, j1, j2); }
GLAPI_WRAPPER void APIENTRY glEvalPoint1(GLint i) { impl_glEvalPoint1(i); }
GLAPI_WRAPPER void APIENTRY glEvalPoint2(GLint i, GLint j) { impl_glEvalPoint2(i, j); }
GLAPI_WRAPPER void APIENTRY glFeedbackBuffer(GLsizei size, GLenum type, GLfloat* buffer) { impl_glFeedbackBuffer(size, type, buffer); }
GLAPI_WRAPPER void APIENTRY glFinish(void) { impl_glFinish(); }
GLAPI_WRAPPER void APIENTRY glFlush(void) { impl_glFlush(); }
GLAPI_WRAPPER void APIENTRY glFogf(GLenum pname, GLfloat param) { impl_glFogf(pname, param); }
GLAPI_WRAPPER void APIENTRY glFogfv(GLenum pname, const GLfloat* params) { impl_glFogfv(pname, params); }
GLAPI_WRAPPER void APIENTRY glFogi(GLenum pname, GLint param) { impl_glFogi(pname, param); }
GLAPI_WRAPPER void APIENTRY glFogiv(GLenum pname, const GLint* params) { impl_glFogiv(pname, params); }
GLAPI_WRAPPER void APIENTRY glFrontFace(GLenum mode) { impl_glFrontFace(mode); }
GLAPI_WRAPPER void APIENTRY glFrustum(GLdouble left, GLdouble right, GLdouble bottom, GLdouble top, GLdouble zNear, GLdouble zFar) { impl_glFrustum(left, right, bottom, top, zNear, zFar); }
GLAPI_WRAPPER void APIENTRY glFrustumf(GLfloat left, GLfloat right, GLfloat bottom, GLfloat top, GLfloat zNear, GLfloat zFar) { impl_glFrustumf(left, right, bottom, top, zNear, zFar); }
GLAPI_WRAPPER GLuint APIENTRY glGenLists(GLsizei range) { return impl_glGenLists(range); }
GLAPI_WRAPPER void APIENTRY glGetBooleanv(GLenum pname, GLboolean* params) { impl_glGetBooleanv(pname, params); }
GLAPI_WRAPPER void APIENTRY glGetClipPlane(GLenum plane, GLdouble* equation) { impl_glGetClipPlane(plane, equation); }
GLAPI_WRAPPER void APIENTRY glGetClipPlanef(GLenum plane, GLfloat* equation) { impl_glGetClipPlanef(plane, equation); }
GLAPI_WRAPPER void APIENTRY glGetDoublev(GLenum pname, GLdouble* params) { impl_glGetDoublev(pname, params); }
GLAPI_WRAPPER GLenum APIENTRY glGetError(void) { return impl_glGetError(); }
GLAPI_WRAPPER void APIENTRY glGetFloatv(GLenum pname, GLfloat* params) { impl_glGetFloatv(pname, params); }
GLAPI_WRAPPER void APIENTRY glGetIntegerv(GLenum pname, GLint* params) { impl_glGetIntegerv(pname, params); }
GLAPI_WRAPPER void APIENTRY glGetLightfv(GLenum light, GLenum pname, GLfloat* params) { impl_glGetLightfv(light, pname, params); }
GLAPI_WRAPPER void APIENTRY glGetLightiv(GLenum light, GLenum pname, GLint* params) { impl_glGetLightiv(light, pname, params); }
GLAPI_WRAPPER void APIENTRY glGetMapdv(GLenum target, GLenum query, GLdouble* v) { impl_glGetMapdv(target, query, v); }
GLAPI_WRAPPER void APIENTRY glGetMapfv(GLenum target, GLenum query, GLfloat* v) { impl_glGetMapfv(target, query, v); }
GLAPI_WRAPPER void APIENTRY glGetMapiv(GLenum target, GLenum query, GLint* v) { impl_glGetMapiv(target, query, v); }
GLAPI_WRAPPER void APIENTRY glGetMaterialfv(GLenum face, GLenum pname, GLfloat* params) { impl_glGetMaterialfv(face, pname, params); }
GLAPI_WRAPPER void APIENTRY glGetMaterialiv(GLenum face, GLenum pname, GLint* params) { impl_glGetMaterialiv(face, pname, params); }
GLAPI_WRAPPER void APIENTRY glGetPixelMapfv(GLenum map, GLfloat* values) { impl_glGetPixelMapfv(map, values); }
GLAPI_WRAPPER void APIENTRY glGetPixelMapuiv(GLenum map, GLuint* values) { impl_glGetPixelMapuiv(map, values); }
GLAPI_WRAPPER void APIENTRY glGetPixelMapusv(GLenum map, GLushort* values) { impl_glGetPixelMapusv(map, values); }
GLAPI_WRAPPER void APIENTRY glGetPolygonStipple(GLubyte* mask) { impl_glGetPolygonStipple(mask); }
GLAPI_WRAPPER const GLubyte* APIENTRY glGetString(GLenum name) { return impl_glGetString(name); }
GLAPI_WRAPPER void APIENTRY glGetTexEnvfv(GLenum target, GLenum pname, GLfloat* params) { impl_glGetTexEnvfv(target, pname, params); }
GLAPI_WRAPPER void APIENTRY glGetTexEnviv(GLenum target, GLenum pname, GLint* params) { impl_glGetTexEnviv(target, pname, params); }
GLAPI_WRAPPER void APIENTRY glGetTexGendv(GLenum coord, GLenum pname, GLdouble* params) { impl_glGetTexGendv(coord, pname, params); }
GLAPI_WRAPPER void APIENTRY glGetTexGenfv(GLenum coord, GLenum pname, GLfloat* params) { impl_glGetTexGenfv(coord, pname, params); }
GLAPI_WRAPPER void APIENTRY glGetTexGeniv(GLenum coord, GLenum pname, GLint* params) { impl_glGetTexGeniv(coord, pname, params); }
GLAPI_WRAPPER void APIENTRY glGetTexImage(GLenum target, GLint level, GLenum format, GLenum type, GLvoid* pixels) { impl_glGetTexImage(target, level, format, type, pixels); }
GLAPI_WRAPPER void APIENTRY glGetTexLevelParameterfv(GLenum target, GLint level, GLenum pname, GLfloat* params) { impl_glGetTexLevelParameterfv(target, level, pname, params); }
GLAPI_WRAPPER void APIENTRY glGetTexLevelParameteriv(GLenum target, GLint level, GLenum pname, GLint* params) { impl_glGetTexLevelParameteriv(target, level, pname, params); }
GLAPI_WRAPPER void APIENTRY glGetTexParameterfv(GLenum target, GLenum pname, GLfloat* params) { impl_glGetTexParameterfv(target, pname, params); }
GLAPI_WRAPPER void APIENTRY glGetTexParameteriv(GLenum target, GLenum pname, GLint* params) { impl_glGetTexParameteriv(target, pname, params); }
GLAPI_WRAPPER void APIENTRY glHint(GLenum target, GLenum mode) { impl_glHint(target, mode); }
GLAPI_WRAPPER void APIENTRY glIndexMask(GLuint mask) { impl_glIndexMask(mask); }
GLAPI_WRAPPER void APIENTRY glIndexd(GLdouble c) { impl_glIndexd(c); }
GLAPI_WRAPPER void APIENTRY glIndexdv(const GLdouble* c) { impl_glIndexdv(c); }
GLAPI_WRAPPER void APIENTRY glIndexf(GLfloat c) { impl_glIndexf(c); }
GLAPI_WRAPPER void APIENTRY glIndexfv(const GLfloat* c) { impl_glIndexfv(c); }
GLAPI_WRAPPER void APIENTRY glIndexi(GLint c) { impl_glIndexi(c); }
GLAPI_WRAPPER void APIENTRY glIndexiv(const GLint* c) { impl_glIndexiv(c); }
GLAPI_WRAPPER void APIENTRY glIndexs(GLshort c) { impl_glIndexs(c); }
GLAPI_WRAPPER void APIENTRY glIndexsv(const GLshort* c) { impl_glIndexsv(c); }
GLAPI_WRAPPER void APIENTRY glInitNames(void) { impl_glInitNames(); }
GLAPI_WRAPPER GLboolean APIENTRY glIsEnabled(GLenum cap) { return impl_glIsEnabled(cap); }
GLAPI_WRAPPER GLboolean APIENTRY glIsList(GLuint list) { return impl_glIsList(list); }
GLAPI_WRAPPER void APIENTRY glLightModelf(GLenum pname, GLfloat param) { impl_glLightModelf(pname, param); }
GLAPI_WRAPPER void APIENTRY glLightModelfv(GLenum pname, const GLfloat* params) { impl_glLightModelfv(pname, params); }
GLAPI_WRAPPER void APIENTRY glLightModeli(GLenum pname, GLint param) { impl_glLightModeli(pname, param); }
GLAPI_WRAPPER void APIENTRY glLightModeliv(GLenum pname, const GLint* params) { impl_glLightModeliv(pname, params); }
GLAPI_WRAPPER void APIENTRY glLightf(GLenum light, GLenum pname, GLfloat param) { impl_glLightf(light, pname, param); }
GLAPI_WRAPPER void APIENTRY glLightfv(GLenum light, GLenum pname, const GLfloat* params) { impl_glLightfv(light, pname, params); }
GLAPI_WRAPPER void APIENTRY glLighti(GLenum light, GLenum pname, GLint param) { impl_glLighti(light, pname, param); }
GLAPI_WRAPPER void APIENTRY glLightiv(GLenum light, GLenum pname, const GLint* params) { impl_glLightiv(light, pname, params); }
GLAPI_WRAPPER void APIENTRY glLineStipple(GLint factor, GLushort pattern) { impl_glLineStipple(factor, pattern); }
GLAPI_WRAPPER void APIENTRY glLineWidth(GLfloat width) { impl_glLineWidth(width); }
GLAPI_WRAPPER void APIENTRY glListBase(GLuint base) { impl_glListBase(base); }
GLAPI_WRAPPER void APIENTRY glLoadIdentity(void) { impl_glLoadIdentity(); }
GLAPI_WRAPPER void APIENTRY glLoadMatrixd(const GLdouble* m) { impl_glLoadMatrixd(m); }
GLAPI_WRAPPER void APIENTRY glLoadMatrixf(const GLfloat* m) { impl_glLoadMatrixf(m); }
GLAPI_WRAPPER void APIENTRY glLoadName(GLuint name) { impl_glLoadName(name); }
GLAPI_WRAPPER void APIENTRY glLogicOp(GLenum opcode) { impl_glLogicOp(opcode); }
GLAPI_WRAPPER void APIENTRY glMap1d(GLenum target, GLdouble u1, GLdouble u2, GLint stride, GLint order, const GLdouble* points) { impl_glMap1d(target, u1, u2, stride, order, points); }
GLAPI_WRAPPER void APIENTRY glMap1f(GLenum target, GLfloat u1, GLfloat u2, GLint stride, GLint order, const GLfloat* points) { impl_glMap1f(target, u1, u2, stride, order, points); }
GLAPI_WRAPPER void APIENTRY glMap2d(GLenum target, GLdouble u1, GLdouble u2, GLint ustride, GLint uorder, GLdouble v1, GLdouble v2, GLint vstride, GLint vorder, const GLdouble* points) { impl_glMap2d(target, u1, u2, ustride, uorder, v1, v2, vstride, vorder, points); }
GLAPI_WRAPPER void APIENTRY glMap2f(GLenum target, GLfloat u1, GLfloat u2, GLint ustride, GLint uorder, GLfloat v1, GLfloat v2, GLint vstride, GLint vorder, const GLfloat* points) { impl_glMap2f(target, u1, u2, ustride, uorder, v1, v2, vstride, vorder, points); }
GLAPI_WRAPPER void APIENTRY glMapGrid1d(GLint un, GLdouble u1, GLdouble u2) { impl_glMapGrid1d(un, u1, u2); }
GLAPI_WRAPPER void APIENTRY glMapGrid1f(GLint un, GLfloat u1, GLfloat u2) { impl_glMapGrid1f(un, u1, u2); }
GLAPI_WRAPPER void APIENTRY glMapGrid2d(GLint un, GLdouble u1, GLdouble u2, GLint vn, GLdouble v1, GLdouble v2) { impl_glMapGrid2d(un, u1, u2, vn, v1, v2); }
GLAPI_WRAPPER void APIENTRY glMapGrid2f(GLint un, GLfloat u1, GLfloat u2, GLint vn, GLfloat v1, GLfloat v2) { impl_glMapGrid2f(un, u1, u2, vn, v1, v2); }
GLAPI_WRAPPER void APIENTRY glMaterialf(GLenum face, GLenum pname, GLfloat param) { impl_glMaterialf(face, pname, param); }
GLAPI_WRAPPER void APIENTRY glMaterialfv(GLenum face, GLenum pname, const GLfloat* params) { impl_glMaterialfv(face, pname, params); }
GLAPI_WRAPPER void APIENTRY glMateriali(GLenum face, GLenum pname, GLint param) { impl_glMateriali(face, pname, param); }
GLAPI_WRAPPER void APIENTRY glMaterialiv(GLenum face, GLenum pname, const GLint* params) { impl_glMaterialiv(face, pname, params); }
GLAPI_WRAPPER void APIENTRY glMatrixMode(GLenum mode) { impl_glMatrixMode(mode); }
GLAPI_WRAPPER void APIENTRY glMultMatrixd(const GLdouble* m) { impl_glMultMatrixd(m); }
GLAPI_WRAPPER void APIENTRY glMultMatrixf(const GLfloat* m) { impl_glMultMatrixf(m); }
GLAPI_WRAPPER void APIENTRY glNewList(GLuint list, GLenum mode) { impl_glNewList(list, mode); }
GLAPI_WRAPPER void APIENTRY glNormal3b(GLbyte nx, GLbyte ny, GLbyte nz) { impl_glNormal3b(nx, ny, nz); }
GLAPI_WRAPPER void APIENTRY glNormal3bv(const GLbyte* v) { impl_glNormal3bv(v); }
GLAPI_WRAPPER void APIENTRY glNormal3d(GLdouble nx, GLdouble ny, GLdouble nz) { impl_glNormal3d(nx, ny, nz); }
GLAPI_WRAPPER void APIENTRY glNormal3dv(const GLdouble* v) { impl_glNormal3dv(v); }
GLAPI_WRAPPER void APIENTRY glNormal3f(GLfloat nx, GLfloat ny, GLfloat nz) { impl_glNormal3f(nx, ny, nz); }
GLAPI_WRAPPER void APIENTRY glNormal3fv(const GLfloat* v) { impl_glNormal3fv(v); }
GLAPI_WRAPPER void APIENTRY glNormal3i(GLint nx, GLint ny, GLint nz) { impl_glNormal3i(nx, ny, nz); }
GLAPI_WRAPPER void APIENTRY glNormal3iv(const GLint* v) { impl_glNormal3iv(v); }
GLAPI_WRAPPER void APIENTRY glNormal3s(GLshort nx, GLshort ny, GLshort nz) { impl_glNormal3s(nx, ny, nz); }
GLAPI_WRAPPER void APIENTRY glNormal3sv(const GLshort* v) { impl_glNormal3sv(v); }
GLAPI_WRAPPER void APIENTRY glOrtho(GLdouble left, GLdouble right, GLdouble bottom, GLdouble top, GLdouble zNear, GLdouble zFar) { impl_glOrtho(left, right, bottom, top, zNear, zFar); }
GLAPI_WRAPPER void APIENTRY glPassThrough(GLfloat token) { impl_glPassThrough(token); }
GLAPI_WRAPPER void APIENTRY glPixelMapfv(GLenum map, GLsizei mapsize, const GLfloat* values) { impl_glPixelMapfv(map, mapsize, values); }
GLAPI_WRAPPER void APIENTRY glPixelMapuiv(GLenum map, GLsizei mapsize, const GLuint* values) { impl_glPixelMapuiv(map, mapsize, values); }
GLAPI_WRAPPER void APIENTRY glPixelMapusv(GLenum map, GLsizei mapsize, const GLushort* values) { impl_glPixelMapusv(map, mapsize, values); }
GLAPI_WRAPPER void APIENTRY glPixelStoref(GLenum pname, GLfloat param) { impl_glPixelStoref(pname, param); }
GLAPI_WRAPPER void APIENTRY glPixelStorei(GLenum pname, GLint param) { impl_glPixelStorei(pname, param); }
GLAPI_WRAPPER void APIENTRY glPixelTransferf(GLenum pname, GLfloat param) { impl_glPixelTransferf(pname, param); }
GLAPI_WRAPPER void APIENTRY glPixelTransferi(GLenum pname, GLint param) { impl_glPixelTransferi(pname, param); }
GLAPI_WRAPPER void APIENTRY glPixelZoom(GLfloat xfactor, GLfloat yfactor) { impl_glPixelZoom(xfactor, yfactor); }
GLAPI_WRAPPER void APIENTRY glPointSize(GLfloat size) { impl_glPointSize(size); }
GLAPI_WRAPPER void APIENTRY glPolygonMode(GLenum face, GLenum mode) { impl_glPolygonMode(face, mode); }
GLAPI_WRAPPER void APIENTRY glPolygonStipple(const GLubyte* mask) { impl_glPolygonStipple(mask); }
GLAPI_WRAPPER void APIENTRY glPopAttrib(void) { impl_glPopAttrib(); }
GLAPI_WRAPPER void APIENTRY glPopMatrix(void) { impl_glPopMatrix(); }
GLAPI_WRAPPER void APIENTRY glPopName(void) { impl_glPopName(); }
GLAPI_WRAPPER void APIENTRY glPushAttrib(GLbitfield mask) { impl_glPushAttrib(mask); }
GLAPI_WRAPPER void APIENTRY glPushMatrix(void) { impl_glPushMatrix(); }
GLAPI_WRAPPER void APIENTRY glPushName(GLuint name) { impl_glPushName(name); }
GLAPI_WRAPPER void APIENTRY glRasterPos2d(GLdouble x, GLdouble y) { impl_glRasterPos2d(x, y); }
GLAPI_WRAPPER void APIENTRY glRasterPos2dv(const GLdouble* v) { impl_glRasterPos2dv(v); }
GLAPI_WRAPPER void APIENTRY glRasterPos2f(GLfloat x, GLfloat y) { impl_glRasterPos2f(x, y); }
GLAPI_WRAPPER void APIENTRY glRasterPos2fv(const GLfloat* v) { impl_glRasterPos2fv(v); }
GLAPI_WRAPPER void APIENTRY glRasterPos2i(GLint x, GLint y) { impl_glRasterPos2i(x, y); }
GLAPI_WRAPPER void APIENTRY glRasterPos2iv(const GLint* v) { impl_glRasterPos2iv(v); }
GLAPI_WRAPPER void APIENTRY glRasterPos2s(GLshort x, GLshort y) { impl_glRasterPos2s(x, y); }
GLAPI_WRAPPER void APIENTRY glRasterPos2sv(const GLshort* v) { impl_glRasterPos2sv(v); }
GLAPI_WRAPPER void APIENTRY glRasterPos3d(GLdouble x, GLdouble y, GLdouble z) { impl_glRasterPos3d(x, y, z); }
GLAPI_WRAPPER void APIENTRY glRasterPos3dv(const GLdouble* v) { impl_glRasterPos3dv(v); }
GLAPI_WRAPPER void APIENTRY glRasterPos3f(GLfloat x, GLfloat y, GLfloat z) { impl_glRasterPos3f(x, y, z); }
GLAPI_WRAPPER void APIENTRY glRasterPos3fv(const GLfloat* v) { impl_glRasterPos3fv(v); }
GLAPI_WRAPPER void APIENTRY glRasterPos3i(GLint x, GLint y, GLint z) { impl_glRasterPos3i(x, y, z); }
GLAPI_WRAPPER void APIENTRY glRasterPos3iv(const GLint* v) { impl_glRasterPos3iv(v); }
GLAPI_WRAPPER void APIENTRY glRasterPos3s(GLshort x, GLshort y, GLshort z) { impl_glRasterPos3s(x, y, z); }
GLAPI_WRAPPER void APIENTRY glRasterPos3sv(const GLshort* v) { impl_glRasterPos3sv(v); }
GLAPI_WRAPPER void APIENTRY glRasterPos4d(GLdouble x, GLdouble y, GLdouble z, GLdouble w) { impl_glRasterPos4d(x, y, z, w); }
GLAPI_WRAPPER void APIENTRY glRasterPos4dv(const GLdouble* v) { impl_glRasterPos4dv(v); }
GLAPI_WRAPPER void APIENTRY glRasterPos4f(GLfloat x, GLfloat y, GLfloat z, GLfloat w) { impl_glRasterPos4f(x, y, z, w); }
GLAPI_WRAPPER void APIENTRY glRasterPos4fv(const GLfloat* v) { impl_glRasterPos4fv(v); }
GLAPI_WRAPPER void APIENTRY glRasterPos4i(GLint x, GLint y, GLint z, GLint w) { impl_glRasterPos4i(x, y, z, w); }
GLAPI_WRAPPER void APIENTRY glRasterPos4iv(const GLint* v) { impl_glRasterPos4iv(v); }
GLAPI_WRAPPER void APIENTRY glRasterPos4s(GLshort x, GLshort y, GLshort z, GLshort w) { impl_glRasterPos4s(x, y, z, w); }
GLAPI_WRAPPER void APIENTRY glRasterPos4sv(const GLshort* v) { impl_glRasterPos4sv(v); }
GLAPI_WRAPPER void APIENTRY glReadBuffer(GLenum mode) { impl_glReadBuffer(mode); }
GLAPI_WRAPPER void APIENTRY glReadPixels(GLint x, GLint y, GLsizei width, GLsizei height, GLenum format, GLenum type, GLvoid* pixels) { impl_glReadPixels(x, y, width, height, format, type, pixels); }
GLAPI_WRAPPER void APIENTRY glRectd(GLdouble x1, GLdouble y1, GLdouble x2, GLdouble y2) { impl_glRectd(x1, y1, x2, y2); }
GLAPI_WRAPPER void APIENTRY glRectdv(const GLdouble* v1, const GLdouble* v2) { impl_glRectdv(v1, v2); }
GLAPI_WRAPPER void APIENTRY glRectf(GLfloat x1, GLfloat y1, GLfloat x2, GLfloat y2) { impl_glRectf(x1, y1, x2, y2); }
GLAPI_WRAPPER void APIENTRY glRectfv(const GLfloat* v1, const GLfloat* v2) { impl_glRectfv(v1, v2); }
GLAPI_WRAPPER void APIENTRY glRecti(GLint x1, GLint y1, GLint x2, GLint y2) { impl_glRecti(x1, y1, x2, y2); }
GLAPI_WRAPPER void APIENTRY glRectiv(const GLint* v1, const GLint* v2) { impl_glRectiv(v1, v2); }
GLAPI_WRAPPER void APIENTRY glRects(GLshort x1, GLshort y1, GLshort x2, GLshort y2) { impl_glRects(x1, y1, x2, y2); }
GLAPI_WRAPPER void APIENTRY glRectsv(const GLshort* v1, const GLshort* v2) { impl_glRectsv(v1, v2); }
GLAPI_WRAPPER GLint APIENTRY glRenderMode(GLenum mode) { return impl_glRenderMode(mode); }
GLAPI_WRAPPER void APIENTRY glRotated(GLdouble angle, GLdouble x, GLdouble y, GLdouble z) { impl_glRotated(angle, x, y, z); }
GLAPI_WRAPPER void APIENTRY glRotatef(GLfloat angle, GLfloat x, GLfloat y, GLfloat z) { impl_glRotatef(angle, x, y, z); }
GLAPI_WRAPPER void APIENTRY glScaled(GLdouble x, GLdouble y, GLdouble z) { impl_glScaled(x, y, z); }
GLAPI_WRAPPER void APIENTRY glScalef(GLfloat x, GLfloat y, GLfloat z) { impl_glScalef(x, y, z); }
GLAPI_WRAPPER void APIENTRY glScissor(GLint x, GLint y, GLsizei width, GLsizei height) { impl_glScissor(x, y, width, height); }
GLAPI_WRAPPER void APIENTRY glSelectBuffer(GLsizei size, GLuint* buffer) { impl_glSelectBuffer(size, buffer); }
GLAPI_WRAPPER void APIENTRY glShadeModel(GLenum mode) { impl_glShadeModel(mode); }
GLAPI_WRAPPER void APIENTRY glStencilFunc(GLenum func, GLint ref, GLuint mask) { impl_glStencilFunc(func, ref, mask); }
GLAPI_WRAPPER void APIENTRY glStencilMask(GLuint mask) { impl_glStencilMask(mask); }
GLAPI_WRAPPER void APIENTRY glStencilOp(GLenum fail, GLenum zfail, GLenum zpass) { impl_glStencilOp(fail, zfail, zpass); }
GLAPI_WRAPPER void APIENTRY glTexCoord1d(GLdouble s) { impl_glTexCoord1d(s); }
GLAPI_WRAPPER void APIENTRY glTexCoord1dv(const GLdouble* v) { impl_glTexCoord1dv(v); }
GLAPI_WRAPPER void APIENTRY glTexCoord1f(GLfloat s) { impl_glTexCoord1f(s); }
GLAPI_WRAPPER void APIENTRY glTexCoord1fv(const GLfloat* v) { impl_glTexCoord1fv(v); }
GLAPI_WRAPPER void APIENTRY glTexCoord1i(GLint s) { impl_glTexCoord1i(s); }
GLAPI_WRAPPER void APIENTRY glTexCoord1iv(const GLint* v) { impl_glTexCoord1iv(v); }
GLAPI_WRAPPER void APIENTRY glTexCoord1s(GLshort s) { impl_glTexCoord1s(s); }
GLAPI_WRAPPER void APIENTRY glTexCoord1sv(const GLshort* v) { impl_glTexCoord1sv(v); }
GLAPI_WRAPPER void APIENTRY glTexCoord2d(GLdouble s, GLdouble t) { impl_glTexCoord2d(s, t); }
GLAPI_WRAPPER void APIENTRY glTexCoord2dv(const GLdouble* v) { impl_glTexCoord2dv(v); }
GLAPI_WRAPPER void APIENTRY glTexCoord2f(GLfloat s, GLfloat t) { impl_glTexCoord2f(s, t); }
GLAPI_WRAPPER void APIENTRY glTexCoord2fv(const GLfloat* v) { impl_glTexCoord2fv(v); }
GLAPI_WRAPPER void APIENTRY glTexCoord2i(GLint s, GLint t) { impl_glTexCoord2i(s, t); }
GLAPI_WRAPPER void APIENTRY glTexCoord2iv(const GLint* v) { impl_glTexCoord2iv(v); }
GLAPI_WRAPPER void APIENTRY glTexCoord2s(GLshort s, GLshort t) { impl_glTexCoord2s(s, t); }
GLAPI_WRAPPER void APIENTRY glTexCoord2sv(const GLshort* v) { impl_glTexCoord2sv(v); }
GLAPI_WRAPPER void APIENTRY glTexCoord3d(GLdouble s, GLdouble t, GLdouble r) { impl_glTexCoord3d(s, t, r); }
GLAPI_WRAPPER void APIENTRY glTexCoord3dv(const GLdouble* v) { impl_glTexCoord3dv(v); }
GLAPI_WRAPPER void APIENTRY glTexCoord3f(GLfloat s, GLfloat t, GLfloat r) { impl_glTexCoord3f(s, t, r); }
GLAPI_WRAPPER void APIENTRY glTexCoord3fv(const GLfloat* v) { impl_glTexCoord3fv(v); }
GLAPI_WRAPPER void APIENTRY glTexCoord3i(GLint s, GLint t, GLint r) { impl_glTexCoord3i(s, t, r); }
GLAPI_WRAPPER void APIENTRY glTexCoord3iv(const GLint* v) { impl_glTexCoord3iv(v); }
GLAPI_WRAPPER void APIENTRY glTexCoord3s(GLshort s, GLshort t, GLshort r) { impl_glTexCoord3s(s, t, r); }
GLAPI_WRAPPER void APIENTRY glTexCoord3sv(const GLshort* v) { impl_glTexCoord3sv(v); }
GLAPI_WRAPPER void APIENTRY glTexCoord4d(GLdouble s, GLdouble t, GLdouble r, GLdouble q) { impl_glTexCoord4d(s, t, r, q); }
GLAPI_WRAPPER void APIENTRY glTexCoord4dv(const GLdouble* v) { impl_glTexCoord4dv(v); }
GLAPI_WRAPPER void APIENTRY glTexCoord4f(GLfloat s, GLfloat t, GLfloat r, GLfloat q) { impl_glTexCoord4f(s, t, r, q); }
GLAPI_WRAPPER void APIENTRY glTexCoord4fv(const GLfloat* v) { impl_glTexCoord4fv(v); }
GLAPI_WRAPPER void APIENTRY glTexCoord4i(GLint s, GLint t, GLint r, GLint q) { impl_glTexCoord4i(s, t, r, q); }
GLAPI_WRAPPER void APIENTRY glTexCoord4iv(const GLint* v) { impl_glTexCoord4iv(v); }
GLAPI_WRAPPER void APIENTRY glTexCoord4s(GLshort s, GLshort t, GLshort r, GLshort q) { impl_glTexCoord4s(s, t, r, q); }
GLAPI_WRAPPER void APIENTRY glTexCoord4sv(const GLshort* v) { impl_glTexCoord4sv(v); }
GLAPI_WRAPPER void APIENTRY glTexEnvf(GLenum target, GLenum pname, GLfloat param) { impl_glTexEnvf(target, pname, param); }
GLAPI_WRAPPER void APIENTRY glTexEnvfv(GLenum target, GLenum pname, const GLfloat* params) { impl_glTexEnvfv(target, pname, params); }
GLAPI_WRAPPER void APIENTRY glTexEnvi(GLenum target, GLenum pname, GLint param) { impl_glTexEnvi(target, pname, param); }
GLAPI_WRAPPER void APIENTRY glTexEnviv(GLenum target, GLenum pname, const GLint* params) { impl_glTexEnviv(target, pname, params); }
GLAPI_WRAPPER void APIENTRY glTexGend(GLenum coord, GLenum pname, GLdouble param) { impl_glTexGend(coord, pname, param); }
GLAPI_WRAPPER void APIENTRY glTexGendv(GLenum coord, GLenum pname, const GLdouble* params) { impl_glTexGendv(coord, pname, params); }
GLAPI_WRAPPER void APIENTRY glTexGenf(GLenum coord, GLenum pname, GLfloat param) { impl_glTexGenf(coord, pname, param); }
GLAPI_WRAPPER void APIENTRY glTexGenfv(GLenum coord, GLenum pname, const GLfloat* params) { impl_glTexGenfv(coord, pname, params); }
GLAPI_WRAPPER void APIENTRY glTexGeni(GLenum coord, GLenum pname, GLint param) { impl_glTexGeni(coord, pname, param); }
GLAPI_WRAPPER void APIENTRY glTexGeniv(GLenum coord, GLenum pname, const GLint* params) { impl_glTexGeniv(coord, pname, params); }
GLAPI_WRAPPER void APIENTRY glTexImage1D(GLenum target, GLint level, GLint internalformat, GLsizei width, GLint border, GLenum format, GLenum type, const GLvoid* pixels) { impl_glTexImage1D(target, level, internalformat, width, border, format, type, pixels); }
GLAPI_WRAPPER void APIENTRY glTexImage2D(GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLint border, GLenum format, GLenum type, const GLvoid* pixels) { impl_glTexImage2D(target, level, internalformat, width, height, border, format, type, pixels); }
GLAPI_WRAPPER void APIENTRY glTexParameterf(GLenum target, GLenum pname, GLfloat param) { impl_glTexParameterf(target, pname, param); }
GLAPI_WRAPPER void APIENTRY glTexParameterfv(GLenum target, GLenum pname, const GLfloat* params) { impl_glTexParameterfv(target, pname, params); }
GLAPI_WRAPPER void APIENTRY glTexParameteri(GLenum target, GLenum pname, GLint param) { impl_glTexParameteri(target, pname, param); }
GLAPI_WRAPPER void APIENTRY glTexParameteriv(GLenum target, GLenum pname, const GLint* params) { impl_glTexParameteriv(target, pname, params); }
GLAPI_WRAPPER void APIENTRY glTranslated(GLdouble x, GLdouble y, GLdouble z) { impl_glTranslated(x, y, z); }
GLAPI_WRAPPER void APIENTRY glTranslatef(GLfloat x, GLfloat y, GLfloat z) { impl_glTranslatef(x, y, z); }
GLAPI_WRAPPER void APIENTRY glVertex2d(GLdouble x, GLdouble y) { impl_glVertex2d(x, y); }
GLAPI_WRAPPER void APIENTRY glVertex2dv(const GLdouble* v) { impl_glVertex2dv(v); }
GLAPI_WRAPPER void APIENTRY glVertex2f(GLfloat x, GLfloat y) { impl_glVertex2f(x, y); }
GLAPI_WRAPPER void APIENTRY glVertex2fv(const GLfloat* v) { impl_glVertex2fv(v); }
GLAPI_WRAPPER void APIENTRY glVertex2i(GLint x, GLint y) { impl_glVertex2i(x, y); }
GLAPI_WRAPPER void APIENTRY glVertex2iv(const GLint* v) { impl_glVertex2iv(v); }
GLAPI_WRAPPER void APIENTRY glVertex2s(GLshort x, GLshort y) { impl_glVertex2s(x, y); }
GLAPI_WRAPPER void APIENTRY glVertex2sv(const GLshort* v) { impl_glVertex2sv(v); }
GLAPI_WRAPPER void APIENTRY glVertex3d(GLdouble x, GLdouble y, GLdouble z) { impl_glVertex3d(x, y, z); }
GLAPI_WRAPPER void APIENTRY glVertex3dv(const GLdouble* v) { impl_glVertex3dv(v); }
GLAPI_WRAPPER void APIENTRY glVertex3f(GLfloat x, GLfloat y, GLfloat z) { impl_glVertex3f(x, y, z); }
GLAPI_WRAPPER void APIENTRY glVertex3fv(const GLfloat* v) { impl_glVertex3fv(v); }
GLAPI_WRAPPER void APIENTRY glVertex3i(GLint x, GLint y, GLint z) { impl_glVertex3i(x, y, z); }
GLAPI_WRAPPER void APIENTRY glVertex3iv(const GLint* v) { impl_glVertex3iv(v); }
GLAPI_WRAPPER void APIENTRY glVertex3s(GLshort x, GLshort y, GLshort z) { impl_glVertex3s(x, y, z); }
GLAPI_WRAPPER void APIENTRY glVertex3sv(const GLshort* v) { impl_glVertex3sv(v); }
GLAPI_WRAPPER void APIENTRY glVertex4d(GLdouble x, GLdouble y, GLdouble z, GLdouble w) { impl_glVertex4d(x, y, z, w); }
GLAPI_WRAPPER void APIENTRY glVertex4dv(const GLdouble* v) { impl_glVertex4dv(v); }
GLAPI_WRAPPER void APIENTRY glVertex4f(GLfloat x, GLfloat y, GLfloat z, GLfloat w) { impl_glVertex4f(x, y, z, w); }
GLAPI_WRAPPER void APIENTRY glVertex4fv(const GLfloat* v) { impl_glVertex4fv(v); }
GLAPI_WRAPPER void APIENTRY glVertex4i(GLint x, GLint y, GLint z, GLint w) { impl_glVertex4i(x, y, z, w); }
GLAPI_WRAPPER void APIENTRY glVertex4iv(const GLint* v) { impl_glVertex4iv(v); }
GLAPI_WRAPPER void APIENTRY glVertex4s(GLshort x, GLshort y, GLshort z, GLshort w) { impl_glVertex4s(x, y, z, w); }
GLAPI_WRAPPER void APIENTRY glVertex4sv(const GLshort* v) { impl_glVertex4sv(v); }
GLAPI_WRAPPER void APIENTRY glViewport(GLint x, GLint y, GLsizei width, GLsizei height) { impl_glViewport(x, y, width, height); }
// -------------------------------------------------------

// Open GL 1.1
// -------------------------------------------------------
GLAPI_WRAPPER GLboolean APIENTRY glAreTexturesResident(GLsizei n, const GLuint* textures, GLboolean* residences) { return impl_glAreTexturesResident(n, textures, residences); }
GLAPI_WRAPPER void APIENTRY glArrayElement(GLint i) { impl_glArrayElement(i); }
GLAPI_WRAPPER void APIENTRY glBindTexture(GLenum target, GLuint texture) { impl_glBindTexture(target, texture); }
GLAPI_WRAPPER void APIENTRY glColorPointer(GLint size, GLenum type, GLsizei stride, const GLvoid* pointer) { impl_glColorPointer(size, type, stride, pointer); }
GLAPI_WRAPPER void APIENTRY glCopyTexImage1D(GLenum target, GLint level, GLenum internalFormat, GLint x, GLint y, GLsizei width, GLint border) { impl_glCopyTexImage1D(target, level, internalFormat, x, y, width, border); }
GLAPI_WRAPPER void APIENTRY glCopyTexImage2D(GLenum target, GLint level, GLenum internalFormat, GLint x, GLint y, GLsizei width, GLsizei height, GLint border) { impl_glCopyTexImage2D(target, level, internalFormat, x, y, width, height, border); }
GLAPI_WRAPPER void APIENTRY glCopyTexSubImage1D(GLenum target, GLint level, GLint xoffset, GLint x, GLint y, GLsizei width) { impl_glCopyTexSubImage1D(target, level, xoffset, x, y, width); }
GLAPI_WRAPPER void APIENTRY glCopyTexSubImage2D(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint x, GLint y, GLsizei width, GLsizei height) { impl_glCopyTexSubImage2D(target, level, xoffset, yoffset, x, y, width, height); }
GLAPI_WRAPPER void APIENTRY glDeleteTextures(GLsizei n, const GLuint* textures) { impl_glDeleteTextures(n, textures); }
GLAPI_WRAPPER void APIENTRY glDisableClientState(GLenum cap) { impl_glDisableClientState(cap); }
GLAPI_WRAPPER void APIENTRY glDrawArrays(GLenum mode, GLint first, GLsizei count) { impl_glDrawArrays(mode, first, count); }
GLAPI_WRAPPER void APIENTRY glDrawElements(GLenum mode, GLsizei count, GLenum type, const GLvoid* indices) { impl_glDrawElements(mode, count, type, indices); }
GLAPI_WRAPPER void APIENTRY glEdgeFlagPointer(GLsizei stride, const GLvoid* pointer) { impl_glEdgeFlagPointer(stride, pointer); }
GLAPI_WRAPPER void APIENTRY glEnableClientState(GLenum cap) { impl_glEnableClientState(cap); }
GLAPI_WRAPPER void APIENTRY glGenTextures(GLsizei n, GLuint* textures) { impl_glGenTextures(n, textures); }
GLAPI_WRAPPER void APIENTRY glGetPointerv(GLenum pname, GLvoid** params) { impl_glGetPointerv(pname, params); }
GLAPI_WRAPPER GLboolean APIENTRY glIsTexture(GLuint texture) { return impl_glIsTexture(texture); }
GLAPI_WRAPPER void APIENTRY glIndexPointer(GLenum type, GLsizei stride, const GLvoid* pointer) { impl_glIndexPointer(type, stride, pointer); }
GLAPI_WRAPPER void APIENTRY glIndexub(GLubyte c) { impl_glIndexub(c); }
GLAPI_WRAPPER void APIENTRY glIndexubv(const GLubyte* c) { impl_glIndexubv(c); }
GLAPI_WRAPPER void APIENTRY glInterleavedArrays(GLenum format, GLsizei stride, const GLvoid* pointer) { impl_glInterleavedArrays(format, stride, pointer); }
GLAPI_WRAPPER void APIENTRY glNormalPointer(GLenum type, GLsizei stride, const GLvoid* pointer) { impl_glNormalPointer(type, stride, pointer); }
GLAPI_WRAPPER void APIENTRY glPolygonOffset(GLfloat factor, GLfloat units) { impl_glPolygonOffset(factor, units); }
GLAPI_WRAPPER void APIENTRY glPopClientAttrib(void) { impl_glPopClientAttrib(); }
GLAPI_WRAPPER void APIENTRY glPrioritizeTextures(GLsizei n, const GLuint* textures, const GLclampf* priorities) { impl_glPrioritizeTextures(n, textures, priorities); }
GLAPI_WRAPPER void APIENTRY glPushClientAttrib(GLbitfield mask) { impl_glPushClientAttrib(mask); }
GLAPI_WRAPPER void APIENTRY glTexCoordPointer(GLint size, GLenum type, GLsizei stride, const GLvoid* pointer) { impl_glTexCoordPointer(size, type, stride, pointer); }
GLAPI_WRAPPER void APIENTRY glTexSubImage1D(GLenum target, GLint level, GLint xoffset, GLsizei width, GLenum format, GLenum type, const GLvoid* pixels) { impl_glTexSubImage1D(target, level, xoffset, width, format, type, pixels); }
GLAPI_WRAPPER void APIENTRY glTexSubImage2D(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLenum type, const GLvoid* pixels) { impl_glTexSubImage2D(target, level, xoffset, yoffset, width, height, format, type, pixels); }
GLAPI_WRAPPER void APIENTRY glVertexPointer(GLint size, GLenum type, GLsizei stride, const GLvoid* pointer) { impl_glVertexPointer(size, type, stride, pointer); }
// -------------------------------------------------------
// Open GL 1.2
// -------------------------------------------------------
GLAPI_WRAPPER void APIENTRY glDrawRangeElements(GLenum mode, GLuint start, GLuint end, GLsizei count, GLenum type, const GLvoid* indices) { impl_glDrawRangeElements(mode, start, end, count, type, indices); }
GLAPI_WRAPPER void APIENTRY glTexImage3D(GLenum target, GLint level, GLint internalFormat, GLsizei width, GLsizei height, GLsizei depth, GLint border, GLenum format, GLenum type, const GLvoid* data) { impl_glTexImage3D(target, level, internalFormat, width, height, depth, border, format, type, data); }
GLAPI_WRAPPER void APIENTRY glTexSubImage3D(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLenum type, const GLvoid* pixels) { impl_glTexSubImage3D(target, level, xoffset, yoffset, zoffset, width, height, depth, format, type, pixels); }
GLAPI_WRAPPER void APIENTRY glCopyTexSubImage3D(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLint x, GLint y, GLsizei width, GLsizei height) { impl_glCopyTexSubImage3D(target, level, xoffset, yoffset, zoffset, x, y, width, height); }
// -------------------------------------------------------
// Open GL 1.3
// -------------------------------------------------------
GLAPI_WRAPPER void APIENTRY glActiveTexture(GLenum texture) { impl_glActiveTexture(texture); }
GLAPI_WRAPPER void APIENTRY glSampleCoverage(GLfloat value, GLboolean invert) { impl_glSampleCoverage(value, invert); }
GLAPI_WRAPPER void APIENTRY glCompressedTexImage3D(GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth, GLint border, GLsizei imageSize, const GLvoid* data) { impl_glCompressedTexImage3D(target, level, internalformat, width, height, depth, border, imageSize, data); }
GLAPI_WRAPPER void APIENTRY glCompressedTexImage2D(GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLint border, GLsizei imageSize, const GLvoid* data) { impl_glCompressedTexImage2D(target, level, internalformat, width, height, border, imageSize, data); }
GLAPI_WRAPPER void APIENTRY glCompressedTexImage1D(GLenum target, GLint level, GLenum internalformat, GLsizei width, GLint border, GLsizei imageSize, const GLvoid* data) { impl_glCompressedTexImage1D(target, level, internalformat, width, border, imageSize, data); }
GLAPI_WRAPPER void APIENTRY glCompressedTexSubImage3D(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLsizei imageSize, const GLvoid* data) { impl_glCompressedTexSubImage3D(target, level, xoffset, yoffset, zoffset, width, height, depth, format, imageSize, data); }
GLAPI_WRAPPER void APIENTRY glCompressedTexSubImage2D(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLsizei imageSize, const GLvoid* data) { impl_glCompressedTexSubImage2D(target, level, xoffset, yoffset, width, height, format, imageSize, data); }
GLAPI_WRAPPER void APIENTRY glCompressedTexSubImage1D(GLenum target, GLint level, GLint xoffset, GLsizei width, GLenum format, GLsizei imageSize, const GLvoid* data) { impl_glCompressedTexSubImage1D(target, level, xoffset, width, format, imageSize, data); }
GLAPI_WRAPPER void APIENTRY glGetCompressedTexImage(GLenum target, GLint level, GLvoid* pixels) { impl_glGetCompressedTexImage(target, level, pixels); }
GLAPI_WRAPPER void APIENTRY glClientActiveTexture(GLenum texture) { impl_glClientActiveTexture(texture); }
GLAPI_WRAPPER void APIENTRY glMultiTexCoord1d(GLenum target, GLdouble s) { impl_glMultiTexCoord1d(target, s); }
GLAPI_WRAPPER void APIENTRY glMultiTexCoord1dv(GLenum target, const GLdouble* v) { impl_glMultiTexCoord1dv(target, v); }
GLAPI_WRAPPER void APIENTRY glMultiTexCoord1f(GLenum target, GLfloat s) { impl_glMultiTexCoord1f(target, s); }
GLAPI_WRAPPER void APIENTRY glMultiTexCoord1fv(GLenum target, const GLfloat* v) { impl_glMultiTexCoord1fv(target, v); }
GLAPI_WRAPPER void APIENTRY glMultiTexCoord1i(GLenum target, GLint s) { impl_glMultiTexCoord1i(target, s); }
GLAPI_WRAPPER void APIENTRY glMultiTexCoord1iv(GLenum target, const GLint* v) { impl_glMultiTexCoord1iv(target, v); }
GLAPI_WRAPPER void APIENTRY glMultiTexCoord1s(GLenum target, GLshort s) { impl_glMultiTexCoord1s(target, s); }
GLAPI_WRAPPER void APIENTRY glMultiTexCoord1sv(GLenum target, const GLshort* v) { impl_glMultiTexCoord1sv(target, v); }
GLAPI_WRAPPER void APIENTRY glMultiTexCoord2d(GLenum target, GLdouble s, GLdouble t) { impl_glMultiTexCoord2d(target, s, t); }
GLAPI_WRAPPER void APIENTRY glMultiTexCoord2dv(GLenum target, const GLdouble* v) { impl_glMultiTexCoord2dv(target, v); }
GLAPI_WRAPPER void APIENTRY glMultiTexCoord2f(GLenum target, GLfloat s, GLfloat t) { impl_glMultiTexCoord2f(target, s, t); }
GLAPI_WRAPPER void APIENTRY glMultiTexCoord2fv(GLenum target, const GLfloat* v) { impl_glMultiTexCoord2fv(target, v); }
GLAPI_WRAPPER void APIENTRY glMultiTexCoord2i(GLenum target, GLint s, GLint t) { impl_glMultiTexCoord2i(target, s, t); }
GLAPI_WRAPPER void APIENTRY glMultiTexCoord2iv(GLenum target, const GLint* v) { impl_glMultiTexCoord2iv(target, v); }
GLAPI_WRAPPER void APIENTRY glMultiTexCoord2s(GLenum target, GLshort s, GLshort t) { impl_glMultiTexCoord2s(target, s, t); }
GLAPI_WRAPPER void APIENTRY glMultiTexCoord2sv(GLenum target, const GLshort* v) { impl_glMultiTexCoord2sv(target, v); }
GLAPI_WRAPPER void APIENTRY glMultiTexCoord3d(GLenum target, GLdouble s, GLdouble t, GLdouble r) { impl_glMultiTexCoord3d(target, s, t, r); }
GLAPI_WRAPPER void APIENTRY glMultiTexCoord3dv(GLenum target, const GLdouble* v) { impl_glMultiTexCoord3dv(target, v); }
GLAPI_WRAPPER void APIENTRY glMultiTexCoord3f(GLenum target, GLfloat s, GLfloat t, GLfloat r) { impl_glMultiTexCoord3f(target, s, t, r); }
GLAPI_WRAPPER void APIENTRY glMultiTexCoord3fv(GLenum target, const GLfloat* v) { impl_glMultiTexCoord3fv(target, v); }
GLAPI_WRAPPER void APIENTRY glMultiTexCoord3i(GLenum target, GLint s, GLint t, GLint r) { impl_glMultiTexCoord3i(target, s, t, r); }
GLAPI_WRAPPER void APIENTRY glMultiTexCoord3iv(GLenum target, const GLint* v) { impl_glMultiTexCoord3iv(target, v); }
GLAPI_WRAPPER void APIENTRY glMultiTexCoord3s(GLenum target, GLshort s, GLshort t, GLshort r) { impl_glMultiTexCoord3s(target, s, t, r); }
GLAPI_WRAPPER void APIENTRY glMultiTexCoord3sv(GLenum target, const GLshort* v) { impl_glMultiTexCoord3sv(target, v); }
GLAPI_WRAPPER void APIENTRY glMultiTexCoord4d(GLenum target, GLdouble s, GLdouble t, GLdouble r, GLdouble q) { impl_glMultiTexCoord4d(target, s, t, r, q); }
GLAPI_WRAPPER void APIENTRY glMultiTexCoord4dv(GLenum target, const GLdouble* v) { impl_glMultiTexCoord4dv(target, v); }
GLAPI_WRAPPER void APIENTRY glMultiTexCoord4f(GLenum target, GLfloat s, GLfloat t, GLfloat r, GLfloat q) { impl_glMultiTexCoord4f(target, s, t, r, q); }
GLAPI_WRAPPER void APIENTRY glMultiTexCoord4fv(GLenum target, const GLfloat* v) { impl_glMultiTexCoord4fv(target, v); }
GLAPI_WRAPPER void APIENTRY glMultiTexCoord4i(GLenum target, GLint s, GLint t, GLint r, GLint q) { impl_glMultiTexCoord4i(target, s, t, r, q); }
GLAPI_WRAPPER void APIENTRY glMultiTexCoord4iv(GLenum target, const GLint* v) { impl_glMultiTexCoord4iv(target, v); }
GLAPI_WRAPPER void APIENTRY glMultiTexCoord4s(GLenum target, GLshort s, GLshort t, GLshort r, GLshort q) { impl_glMultiTexCoord4s(target, s, t, r, q); }
GLAPI_WRAPPER void APIENTRY glMultiTexCoord4sv(GLenum target, const GLshort* v) { impl_glMultiTexCoord4sv(target, v); }
GLAPI_WRAPPER void APIENTRY glLoadTransposeMatrixf(const GLfloat* m) { impl_glLoadTransposeMatrixf(m); }
GLAPI_WRAPPER void APIENTRY glLoadTransposeMatrixd(const GLdouble* m) { impl_glLoadTransposeMatrixd(m); }
GLAPI_WRAPPER void APIENTRY glMultTransposeMatrixf(const GLfloat* m) { impl_glMultTransposeMatrixf(m); }
GLAPI_WRAPPER void APIENTRY glMultTransposeMatrixd(const GLdouble* m) { impl_glMultTransposeMatrixd(m); }
// -------------------------------------------------------

// Some extensions
// -------------------------------------------------------
GLAPI_WRAPPER void APIENTRY glLockArrays(GLint first, GLsizei count) { impl_glLockArrays(first, count); }
GLAPI_WRAPPER void APIENTRY glUnlockArrays() { impl_glUnlockArrays(); }
GLAPI_WRAPPER void APIENTRY glActiveStencilFaceEXT(GLenum face) { impl_glActiveStencilFaceEXT(face); }
GLAPI_WRAPPER void APIENTRY glBlendEquation(GLenum mode) { impl_glBlendEquation(mode); };
GLAPI_WRAPPER void APIENTRY glBlendFuncSeparate(GLenum sfactorRGB, GLenum dfactorRGB, GLenum sfactorAlpha, GLenum dfactorAlpha) { impl_glBlendFuncSeparate(sfactorRGB, dfactorRGB, sfactorAlpha, dfactorAlpha); }
// -------------------------------------------------------

// OpenGL ES 1.1 extensions
// -------------------------------------------------------
GLAPI_WRAPPER void APIENTRY glPointParameterf(GLenum pname, GLfloat param) { impl_glPointParameterf(pname, param); }
GLAPI_WRAPPER void APIENTRY glPointParameterfv(GLenum pname, const GLfloat* params) { impl_glPointParameterfv(pname, params); }
GLAPI_WRAPPER void APIENTRY glPointSizePointerOES(GLenum type, GLsizei stride, const GLvoid* pointer) { impl_glPointSizePointerOES(type, stride, pointer); }
// -------------------------------------------------------

// Open GL buffer objects
// -------------------------------------------------------
GLAPI_WRAPPER void APIENTRY glBindBuffer(GLenum target, GLuint buffer) { impl_glBindBuffer(target, buffer); }
GLAPI_WRAPPER void APIENTRY glBufferData(GLenum target, GLsizeiptr size, const void* data, GLenum usage) { impl_glBufferData(target, size, data, usage); }
GLAPI_WRAPPER void APIENTRY glBufferSubData(GLenum target, GLintptr offset, GLsizeiptr size, const void* data) { impl_glBufferSubData(target, offset, size, data); }
GLAPI_WRAPPER void APIENTRY glDeleteBuffers(GLsizei n, const GLuint* buffers) { impl_glDeleteBuffers(n, buffers); }
GLAPI_WRAPPER void APIENTRY glGenBuffers(GLsizei n, GLuint* buffers) { impl_glGenBuffers(n, buffers); }
GLAPI_WRAPPER void APIENTRY glGetBufferParameteriv(GLenum target, GLenum pname, GLint* params) { impl_glGetBufferParameteriv(target, pname, params); }
GLAPI_WRAPPER GLboolean APIENTRY glIsBuffer(GLuint buffer) { return impl_glIsBuffer(buffer); }
// -------------------------------------------------------
