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

#ifndef GL_IMPL_H
#define GL_IMPL_H

#include "gl.h"

#ifdef __cplusplus
extern "C"
{
#endif

#if defined(_WIN32) && !defined(APIENTRY) && !defined(__CYGWIN__)
#define WIN32_LEAN_AND_MEAN 1
#include <Windows.h>
#endif

#ifndef APIENTRY
#define APIENTRY
#endif
#ifndef GLAPI
#define GLAPI extern
#endif

#ifndef GLAPI_WRAPPER
#define GLAPI_WRAPPER
#endif

    // Open GL 1.0
    // -------------------------------------------------------
    GLAPI void APIENTRY impl_glAccum(GLenum op, GLfloat value);
    GLAPI void APIENTRY impl_glAlphaFunc(GLenum func, GLclampf ref);
    GLAPI void APIENTRY impl_glBegin(GLenum mode);
    GLAPI void APIENTRY impl_glBitmap(GLsizei width, GLsizei height, GLfloat xOrig, GLfloat yOrig, GLfloat xMove, GLfloat yMove, const GLubyte* bitmap);
    GLAPI void APIENTRY impl_glBlendFunc(GLenum srcFactor, GLenum dstFactor);
    GLAPI void APIENTRY impl_glCallList(GLuint list);
    GLAPI void APIENTRY impl_glCallLists(GLsizei n, GLenum type, const GLvoid* lists);
    GLAPI void APIENTRY impl_glClear(GLbitfield mask);
    GLAPI void APIENTRY impl_glClearAccum(GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha);
    GLAPI void APIENTRY impl_glClearColor(GLclampf red, GLclampf green, GLclampf blue, GLclampf alpha);
    GLAPI void APIENTRY impl_glClearDepth(GLclampd depth);
    GLAPI void APIENTRY impl_glClearIndex(GLfloat c);
    GLAPI void APIENTRY impl_glClearStencil(GLint s);
    GLAPI void APIENTRY impl_glClipPlane(GLenum plane, const GLdouble* equation);
    GLAPI void APIENTRY impl_glColor3b(GLbyte red, GLbyte green, GLbyte blue);
    GLAPI void APIENTRY impl_glColor3bv(const GLbyte* v);
    GLAPI void APIENTRY impl_glColor3d(GLdouble red, GLdouble green, GLdouble blue);
    GLAPI void APIENTRY impl_glColor3dv(const GLdouble* v);
    GLAPI void APIENTRY impl_glColor3f(GLfloat red, GLfloat green, GLfloat blue);
    GLAPI void APIENTRY impl_glColor3fv(const GLfloat* v);
    GLAPI void APIENTRY impl_glColor3i(GLint red, GLint green, GLint blue);
    GLAPI void APIENTRY impl_glColor3iv(const GLint* v);
    GLAPI void APIENTRY impl_glColor3s(GLshort red, GLshort green, GLshort blue);
    GLAPI void APIENTRY impl_glColor3sv(const GLshort* v);
    GLAPI void APIENTRY impl_glColor3ub(GLubyte red, GLubyte green, GLubyte blue);
    GLAPI void APIENTRY impl_glColor3ubv(const GLubyte* v);
    GLAPI void APIENTRY impl_glColor3ui(GLuint red, GLuint green, GLuint blue);
    GLAPI void APIENTRY impl_glColor3uiv(const GLuint* v);
    GLAPI void APIENTRY impl_glColor3us(GLushort red, GLushort green, GLushort blue);
    GLAPI void APIENTRY impl_glColor3usv(const GLushort* v);
    GLAPI void APIENTRY impl_glColor4b(GLbyte red, GLbyte green, GLbyte blue, GLbyte alpha);
    GLAPI void APIENTRY impl_glColor4bv(const GLbyte* v);
    GLAPI void APIENTRY impl_glColor4d(GLdouble red, GLdouble green, GLdouble blue, GLdouble alpha);
    GLAPI void APIENTRY impl_glColor4dv(const GLdouble* v);
    GLAPI void APIENTRY impl_glColor4f(GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha);
    GLAPI void APIENTRY impl_glColor4fv(const GLfloat* v);
    GLAPI void APIENTRY impl_glColor4i(GLint red, GLint green, GLint blue, GLint alpha);
    GLAPI void APIENTRY impl_glColor4iv(const GLint* v);
    GLAPI void APIENTRY impl_glColor4s(GLshort red, GLshort green, GLshort blue, GLshort alpha);
    GLAPI void APIENTRY impl_glColor4sv(const GLshort* v);
    GLAPI void APIENTRY impl_glColor4ub(GLubyte red, GLubyte green, GLubyte blue, GLubyte alpha);
    GLAPI void APIENTRY impl_glColor4ubv(const GLubyte* v);
    GLAPI void APIENTRY impl_glColor4ui(GLuint red, GLuint green, GLuint blue, GLuint alpha);
    GLAPI void APIENTRY impl_glColor4uiv(const GLuint* v);
    GLAPI void APIENTRY impl_glColor4us(GLushort red, GLushort green, GLushort blue, GLushort alpha);
    GLAPI void APIENTRY impl_glColor4usv(const GLushort* v);
    GLAPI void APIENTRY impl_glColorMask(GLboolean red, GLboolean green, GLboolean blue, GLboolean alpha);
    GLAPI void APIENTRY impl_glColorMaterial(GLenum face, GLenum mode);
    GLAPI void APIENTRY impl_glCopyPixels(GLint x, GLint y, GLsizei width, GLsizei height, GLenum type);
    GLAPI void APIENTRY impl_glCullFace(GLenum mode);
    GLAPI void APIENTRY impl_glDeleteLists(GLuint list, GLsizei range);
    GLAPI void APIENTRY impl_glDepthFunc(GLenum func);
    GLAPI void APIENTRY impl_glDepthMask(GLboolean flag);
    GLAPI void APIENTRY impl_glDepthRange(GLclampd zNear, GLclampd zFar);
    GLAPI void APIENTRY impl_glDisable(GLenum cap);
    GLAPI void APIENTRY impl_glDrawBuffer(GLenum mode);
    GLAPI void APIENTRY impl_glDrawPixels(GLsizei width, GLsizei height, GLenum format, GLenum type, const GLvoid* pixels);
    GLAPI void APIENTRY impl_glEdgeFlag(GLboolean flag);
    GLAPI void APIENTRY impl_glEdgeFlagv(const GLboolean* flag);
    GLAPI void APIENTRY impl_glEnable(GLenum cap);
    GLAPI void APIENTRY impl_glEnd(void);
    GLAPI void APIENTRY impl_glEndList(void);
    GLAPI void APIENTRY impl_glEvalCoord1d(GLdouble u);
    GLAPI void APIENTRY impl_glEvalCoord1dv(const GLdouble* u);
    GLAPI void APIENTRY impl_glEvalCoord1f(GLfloat u);
    GLAPI void APIENTRY impl_glEvalCoord1fv(const GLfloat* u);
    GLAPI void APIENTRY impl_glEvalCoord2d(GLdouble u, GLdouble v);
    GLAPI void APIENTRY impl_glEvalCoord2dv(const GLdouble* u);
    GLAPI void APIENTRY impl_glEvalCoord2f(GLfloat u, GLfloat v);
    GLAPI void APIENTRY impl_glEvalCoord2fv(const GLfloat* u);
    GLAPI void APIENTRY impl_glEvalMesh1(GLenum mode, GLint i1, GLint i2);
    GLAPI void APIENTRY impl_glEvalMesh2(GLenum mode, GLint i1, GLint i2, GLint j1, GLint j2);
    GLAPI void APIENTRY impl_glEvalPoint1(GLint i);
    GLAPI void APIENTRY impl_glEvalPoint2(GLint i, GLint j);
    GLAPI void APIENTRY impl_glFeedbackBuffer(GLsizei size, GLenum type, GLfloat* buffer);
    GLAPI void APIENTRY impl_glFinish(void);
    GLAPI void APIENTRY impl_glFlush(void);
    GLAPI void APIENTRY impl_glFogf(GLenum pname, GLfloat param);
    GLAPI void APIENTRY impl_glFogfv(GLenum pname, const GLfloat* params);
    GLAPI void APIENTRY impl_glFogi(GLenum pname, GLint param);
    GLAPI void APIENTRY impl_glFogiv(GLenum pname, const GLint* params);
    GLAPI void APIENTRY impl_glFrontFace(GLenum mode);
    GLAPI void APIENTRY impl_glFrustum(GLdouble left, GLdouble right, GLdouble bottom, GLdouble top, GLdouble zNear, GLdouble zFar);
    GLAPI void APIENTRY impl_glFrustumf(GLfloat left, GLfloat right, GLfloat bottom, GLfloat top, GLfloat zNear, GLfloat zFar);
    GLAPI GLuint APIENTRY impl_glGenLists(GLsizei range);
    GLAPI void APIENTRY impl_glGetBooleanv(GLenum pname, GLboolean* params);
    GLAPI void APIENTRY impl_glGetClipPlane(GLenum plane, GLdouble* equation);
    GLAPI void APIENTRY impl_glGetDoublev(GLenum pname, GLdouble* params);
    GLAPI GLenum APIENTRY impl_glGetError(void);
    GLAPI void APIENTRY impl_glGetFloatv(GLenum pname, GLfloat* params);
    GLAPI void APIENTRY impl_glGetIntegerv(GLenum pname, GLint* params);
    GLAPI void APIENTRY impl_glGetLightfv(GLenum light, GLenum pname, GLfloat* params);
    GLAPI void APIENTRY impl_glGetLightiv(GLenum light, GLenum pname, GLint* params);
    GLAPI void APIENTRY impl_glGetMapdv(GLenum target, GLenum query, GLdouble* v);
    GLAPI void APIENTRY impl_glGetMapfv(GLenum target, GLenum query, GLfloat* v);
    GLAPI void APIENTRY impl_glGetMapiv(GLenum target, GLenum query, GLint* v);
    GLAPI void APIENTRY impl_glGetMaterialfv(GLenum face, GLenum pname, GLfloat* params);
    GLAPI void APIENTRY impl_glGetMaterialiv(GLenum face, GLenum pname, GLint* params);
    GLAPI void APIENTRY impl_glGetPixelMapfv(GLenum map, GLfloat* values);
    GLAPI void APIENTRY impl_glGetPixelMapuiv(GLenum map, GLuint* values);
    GLAPI void APIENTRY impl_glGetPixelMapusv(GLenum map, GLushort* values);
    GLAPI void APIENTRY impl_glGetPolygonStipple(GLubyte* mask);
    GLAPI const GLubyte* APIENTRY impl_glGetString(GLenum name);
    GLAPI void APIENTRY impl_glGetTexEnvfv(GLenum target, GLenum pname, GLfloat* params);
    GLAPI void APIENTRY impl_glGetTexEnviv(GLenum target, GLenum pname, GLint* params);
    GLAPI void APIENTRY impl_glGetTexGendv(GLenum coord, GLenum pname, GLdouble* params);
    GLAPI void APIENTRY impl_glGetTexGenfv(GLenum coord, GLenum pname, GLfloat* params);
    GLAPI void APIENTRY impl_glGetTexGeniv(GLenum coord, GLenum pname, GLint* params);
    GLAPI void APIENTRY impl_glGetTexImage(GLenum target, GLint level, GLenum format, GLenum type, GLvoid* pixels);
    GLAPI void APIENTRY impl_glGetTexLevelParameterfv(GLenum target, GLint level, GLenum pname, GLfloat* params);
    GLAPI void APIENTRY impl_glGetTexLevelParameteriv(GLenum target, GLint level, GLenum pname, GLint* params);
    GLAPI void APIENTRY impl_glGetTexParameterfv(GLenum target, GLenum pname, GLfloat* params);
    GLAPI void APIENTRY impl_glGetTexParameteriv(GLenum target, GLenum pname, GLint* params);
    GLAPI void APIENTRY impl_glHint(GLenum target, GLenum mode);
    GLAPI void APIENTRY impl_glIndexMask(GLuint mask);
    GLAPI void APIENTRY impl_glIndexd(GLdouble c);
    GLAPI void APIENTRY impl_glIndexdv(const GLdouble* c);
    GLAPI void APIENTRY impl_glIndexf(GLfloat c);
    GLAPI void APIENTRY impl_glIndexfv(const GLfloat* c);
    GLAPI void APIENTRY impl_glIndexi(GLint c);
    GLAPI void APIENTRY impl_glIndexiv(const GLint* c);
    GLAPI void APIENTRY impl_glIndexs(GLshort c);
    GLAPI void APIENTRY impl_glIndexsv(const GLshort* c);
    GLAPI void APIENTRY impl_glInitNames(void);
    GLAPI GLboolean APIENTRY impl_glIsEnabled(GLenum cap);
    GLAPI GLboolean APIENTRY impl_glIsList(GLuint list);
    GLAPI void APIENTRY impl_glLightModelf(GLenum pname, GLfloat param);
    GLAPI void APIENTRY impl_glLightModelfv(GLenum pname, const GLfloat* params);
    GLAPI void APIENTRY impl_glLightModeli(GLenum pname, GLint param);
    GLAPI void APIENTRY impl_glLightModeliv(GLenum pname, const GLint* params);
    GLAPI void APIENTRY impl_glLightf(GLenum light, GLenum pname, GLfloat param);
    GLAPI void APIENTRY impl_glLightfv(GLenum light, GLenum pname, const GLfloat* params);
    GLAPI void APIENTRY impl_glLighti(GLenum light, GLenum pname, GLint param);
    GLAPI void APIENTRY impl_glLightiv(GLenum light, GLenum pname, const GLint* params);
    GLAPI void APIENTRY impl_glLineStipple(GLint factor, GLushort pattern);
    GLAPI void APIENTRY impl_glLineWidth(GLfloat width);
    GLAPI void APIENTRY impl_glListBase(GLuint base);
    GLAPI void APIENTRY impl_glLoadIdentity(void);
    GLAPI void APIENTRY impl_glLoadMatrixd(const GLdouble* m);
    GLAPI void APIENTRY impl_glLoadMatrixf(const GLfloat* m);
    GLAPI void APIENTRY impl_glLoadName(GLuint name);
    GLAPI void APIENTRY impl_glLogicOp(GLenum opcode);
    GLAPI void APIENTRY impl_glMap1d(GLenum target, GLdouble u1, GLdouble u2, GLint stride, GLint order, const GLdouble* points);
    GLAPI void APIENTRY impl_glMap1f(GLenum target, GLfloat u1, GLfloat u2, GLint stride, GLint order, const GLfloat* points);
    GLAPI void APIENTRY impl_glMap2d(GLenum target, GLdouble u1, GLdouble u2, GLint ustride, GLint uorder, GLdouble v1, GLdouble v2, GLint vstride, GLint vorder, const GLdouble* points);
    GLAPI void APIENTRY impl_glMap2f(GLenum target, GLfloat u1, GLfloat u2, GLint ustride, GLint uorder, GLfloat v1, GLfloat v2, GLint vstride, GLint vorder, const GLfloat* points);
    GLAPI void APIENTRY impl_glMapGrid1d(GLint un, GLdouble u1, GLdouble u2);
    GLAPI void APIENTRY impl_glMapGrid1f(GLint un, GLfloat u1, GLfloat u2);
    GLAPI void APIENTRY impl_glMapGrid2d(GLint un, GLdouble u1, GLdouble u2, GLint vn, GLdouble v1, GLdouble v2);
    GLAPI void APIENTRY impl_glMapGrid2f(GLint un, GLfloat u1, GLfloat u2, GLint vn, GLfloat v1, GLfloat v2);
    GLAPI void APIENTRY impl_glMaterialf(GLenum face, GLenum pname, GLfloat param);
    GLAPI void APIENTRY impl_glMaterialfv(GLenum face, GLenum pname, const GLfloat* params);
    GLAPI void APIENTRY impl_glMateriali(GLenum face, GLenum pname, GLint param);
    GLAPI void APIENTRY impl_glMaterialiv(GLenum face, GLenum pname, const GLint* params);
    GLAPI void APIENTRY impl_glMatrixMode(GLenum mode);
    GLAPI void APIENTRY impl_glMultMatrixd(const GLdouble* m);
    GLAPI void APIENTRY impl_glMultMatrixf(const GLfloat* m);
    GLAPI void APIENTRY impl_glNewList(GLuint list, GLenum mode);
    GLAPI void APIENTRY impl_glNormal3b(GLbyte nx, GLbyte ny, GLbyte nz);
    GLAPI void APIENTRY impl_glNormal3bv(const GLbyte* v);
    GLAPI void APIENTRY impl_glNormal3d(GLdouble nx, GLdouble ny, GLdouble nz);
    GLAPI void APIENTRY impl_glNormal3dv(const GLdouble* v);
    GLAPI void APIENTRY impl_glNormal3f(GLfloat nx, GLfloat ny, GLfloat nz);
    GLAPI void APIENTRY impl_glNormal3fv(const GLfloat* v);
    GLAPI void APIENTRY impl_glNormal3i(GLint nx, GLint ny, GLint nz);
    GLAPI void APIENTRY impl_glNormal3iv(const GLint* v);
    GLAPI void APIENTRY impl_glNormal3s(GLshort nx, GLshort ny, GLshort nz);
    GLAPI void APIENTRY impl_glNormal3sv(const GLshort* v);
    GLAPI void APIENTRY impl_glOrtho(GLdouble left, GLdouble right, GLdouble bottom, GLdouble top, GLdouble zNear, GLdouble zFar);
    GLAPI void APIENTRY impl_glPassThrough(GLfloat token);
    GLAPI void APIENTRY impl_glPixelMapfv(GLenum map, GLsizei mapsize, const GLfloat* values);
    GLAPI void APIENTRY impl_glPixelMapuiv(GLenum map, GLsizei mapsize, const GLuint* values);
    GLAPI void APIENTRY impl_glPixelMapusv(GLenum map, GLsizei mapsize, const GLushort* values);
    GLAPI void APIENTRY impl_glPixelStoref(GLenum pname, GLfloat param);
    GLAPI void APIENTRY impl_glPixelStorei(GLenum pname, GLint param);
    GLAPI void APIENTRY impl_glPixelTransferf(GLenum pname, GLfloat param);
    GLAPI void APIENTRY impl_glPixelTransferi(GLenum pname, GLint param);
    GLAPI void APIENTRY impl_glPixelZoom(GLfloat xfactor, GLfloat yfactor);
    GLAPI void APIENTRY impl_glPointSize(GLfloat size);
    GLAPI void APIENTRY impl_glPolygonMode(GLenum face, GLenum mode);
    GLAPI void APIENTRY impl_glPolygonStipple(const GLubyte* mask);
    GLAPI void APIENTRY impl_glPopAttrib(void);
    GLAPI void APIENTRY impl_glPopMatrix(void);
    GLAPI void APIENTRY impl_glPopName(void);
    GLAPI void APIENTRY impl_glPushAttrib(GLbitfield mask);
    GLAPI void APIENTRY impl_glPushMatrix(void);
    GLAPI void APIENTRY impl_glPushName(GLuint name);
    GLAPI void APIENTRY impl_glRasterPos2d(GLdouble x, GLdouble y);
    GLAPI void APIENTRY impl_glRasterPos2dv(const GLdouble* v);
    GLAPI void APIENTRY impl_glRasterPos2f(GLfloat x, GLfloat y);
    GLAPI void APIENTRY impl_glRasterPos2fv(const GLfloat* v);
    GLAPI void APIENTRY impl_glRasterPos2i(GLint x, GLint y);
    GLAPI void APIENTRY impl_glRasterPos2iv(const GLint* v);
    GLAPI void APIENTRY impl_glRasterPos2s(GLshort x, GLshort y);
    GLAPI void APIENTRY impl_glRasterPos2sv(const GLshort* v);
    GLAPI void APIENTRY impl_glRasterPos3d(GLdouble x, GLdouble y, GLdouble z);
    GLAPI void APIENTRY impl_glRasterPos3dv(const GLdouble* v);
    GLAPI void APIENTRY impl_glRasterPos3f(GLfloat x, GLfloat y, GLfloat z);
    GLAPI void APIENTRY impl_glRasterPos3fv(const GLfloat* v);
    GLAPI void APIENTRY impl_glRasterPos3i(GLint x, GLint y, GLint z);
    GLAPI void APIENTRY impl_glRasterPos3iv(const GLint* v);
    GLAPI void APIENTRY impl_glRasterPos3s(GLshort x, GLshort y, GLshort z);
    GLAPI void APIENTRY impl_glRasterPos3sv(const GLshort* v);
    GLAPI void APIENTRY impl_glRasterPos4d(GLdouble x, GLdouble y, GLdouble z, GLdouble w);
    GLAPI void APIENTRY impl_glRasterPos4dv(const GLdouble* v);
    GLAPI void APIENTRY impl_glRasterPos4f(GLfloat x, GLfloat y, GLfloat z, GLfloat w);
    GLAPI void APIENTRY impl_glRasterPos4fv(const GLfloat* v);
    GLAPI void APIENTRY impl_glRasterPos4i(GLint x, GLint y, GLint z, GLint w);
    GLAPI void APIENTRY impl_glRasterPos4iv(const GLint* v);
    GLAPI void APIENTRY impl_glRasterPos4s(GLshort x, GLshort y, GLshort z, GLshort w);
    GLAPI void APIENTRY impl_glRasterPos4sv(const GLshort* v);
    GLAPI void APIENTRY impl_glReadBuffer(GLenum mode);
    GLAPI void APIENTRY impl_glReadPixels(GLint x, GLint y, GLsizei width, GLsizei height, GLenum format, GLenum type, GLvoid* pixels);
    GLAPI void APIENTRY impl_glRectd(GLdouble x1, GLdouble y1, GLdouble x2, GLdouble y2);
    GLAPI void APIENTRY impl_glRectdv(const GLdouble* v1, const GLdouble* v2);
    GLAPI void APIENTRY impl_glRectf(GLfloat x1, GLfloat y1, GLfloat x2, GLfloat y2);
    GLAPI void APIENTRY impl_glRectfv(const GLfloat* v1, const GLfloat* v2);
    GLAPI void APIENTRY impl_glRecti(GLint x1, GLint y1, GLint x2, GLint y2);
    GLAPI void APIENTRY impl_glRectiv(const GLint* v1, const GLint* v2);
    GLAPI void APIENTRY impl_glRects(GLshort x1, GLshort y1, GLshort x2, GLshort y2);
    GLAPI void APIENTRY impl_glRectsv(const GLshort* v1, const GLshort* v2);
    GLAPI GLint APIENTRY impl_glRenderMode(GLenum mode);
    GLAPI void APIENTRY impl_glRotated(GLdouble angle, GLdouble x, GLdouble y, GLdouble z);
    GLAPI void APIENTRY impl_glRotatef(GLfloat angle, GLfloat x, GLfloat y, GLfloat z);
    GLAPI void APIENTRY impl_glScaled(GLdouble x, GLdouble y, GLdouble z);
    GLAPI void APIENTRY impl_glScalef(GLfloat x, GLfloat y, GLfloat z);
    GLAPI void APIENTRY impl_glScissor(GLint x, GLint y, GLsizei width, GLsizei height);
    GLAPI void APIENTRY impl_glSelectBuffer(GLsizei size, GLuint* buffer);
    GLAPI void APIENTRY impl_glShadeModel(GLenum mode);
    GLAPI void APIENTRY impl_glStencilFunc(GLenum func, GLint ref, GLuint mask);
    GLAPI void APIENTRY impl_glStencilMask(GLuint mask);
    GLAPI void APIENTRY impl_glStencilOp(GLenum fail, GLenum zfail, GLenum zpass);
    GLAPI void APIENTRY impl_glTexCoord1d(GLdouble s);
    GLAPI void APIENTRY impl_glTexCoord1dv(const GLdouble* v);
    GLAPI void APIENTRY impl_glTexCoord1f(GLfloat s);
    GLAPI void APIENTRY impl_glTexCoord1fv(const GLfloat* v);
    GLAPI void APIENTRY impl_glTexCoord1i(GLint s);
    GLAPI void APIENTRY impl_glTexCoord1iv(const GLint* v);
    GLAPI void APIENTRY impl_glTexCoord1s(GLshort s);
    GLAPI void APIENTRY impl_glTexCoord1sv(const GLshort* v);
    GLAPI void APIENTRY impl_glTexCoord2d(GLdouble s, GLdouble t);
    GLAPI void APIENTRY impl_glTexCoord2dv(const GLdouble* v);
    GLAPI void APIENTRY impl_glTexCoord2f(GLfloat s, GLfloat t);
    GLAPI void APIENTRY impl_glTexCoord2fv(const GLfloat* v);
    GLAPI void APIENTRY impl_glTexCoord2i(GLint s, GLint t);
    GLAPI void APIENTRY impl_glTexCoord2iv(const GLint* v);
    GLAPI void APIENTRY impl_glTexCoord2s(GLshort s, GLshort t);
    GLAPI void APIENTRY impl_glTexCoord2sv(const GLshort* v);
    GLAPI void APIENTRY impl_glTexCoord3d(GLdouble s, GLdouble t, GLdouble r);
    GLAPI void APIENTRY impl_glTexCoord3dv(const GLdouble* v);
    GLAPI void APIENTRY impl_glTexCoord3f(GLfloat s, GLfloat t, GLfloat r);
    GLAPI void APIENTRY impl_glTexCoord3fv(const GLfloat* v);
    GLAPI void APIENTRY impl_glTexCoord3i(GLint s, GLint t, GLint r);
    GLAPI void APIENTRY impl_glTexCoord3iv(const GLint* v);
    GLAPI void APIENTRY impl_glTexCoord3s(GLshort s, GLshort t, GLshort r);
    GLAPI void APIENTRY impl_glTexCoord3sv(const GLshort* v);
    GLAPI void APIENTRY impl_glTexCoord4d(GLdouble s, GLdouble t, GLdouble r, GLdouble q);
    GLAPI void APIENTRY impl_glTexCoord4dv(const GLdouble* v);
    GLAPI void APIENTRY impl_glTexCoord4f(GLfloat s, GLfloat t, GLfloat r, GLfloat q);
    GLAPI void APIENTRY impl_glTexCoord4fv(const GLfloat* v);
    GLAPI void APIENTRY impl_glTexCoord4i(GLint s, GLint t, GLint r, GLint q);
    GLAPI void APIENTRY impl_glTexCoord4iv(const GLint* v);
    GLAPI void APIENTRY impl_glTexCoord4s(GLshort s, GLshort t, GLshort r, GLshort q);
    GLAPI void APIENTRY impl_glTexCoord4sv(const GLshort* v);
    GLAPI void APIENTRY impl_glTexEnvf(GLenum target, GLenum pname, GLfloat param);
    GLAPI void APIENTRY impl_glTexEnvfv(GLenum target, GLenum pname, const GLfloat* params);
    GLAPI void APIENTRY impl_glTexEnvi(GLenum target, GLenum pname, GLint param);
    GLAPI void APIENTRY impl_glTexEnviv(GLenum target, GLenum pname, const GLint* params);
    GLAPI void APIENTRY impl_glTexGend(GLenum coord, GLenum pname, GLdouble param);
    GLAPI void APIENTRY impl_glTexGendv(GLenum coord, GLenum pname, const GLdouble* params);
    GLAPI void APIENTRY impl_glTexGenf(GLenum coord, GLenum pname, GLfloat param);
    GLAPI void APIENTRY impl_glTexGenfv(GLenum coord, GLenum pname, const GLfloat* params);
    GLAPI void APIENTRY impl_glTexGeni(GLenum coord, GLenum pname, GLint param);
    GLAPI void APIENTRY impl_glTexGeniv(GLenum coord, GLenum pname, const GLint* params);
    GLAPI void APIENTRY impl_glTexImage1D(GLenum target, GLint level, GLint internalformat, GLsizei width, GLint border, GLenum format, GLenum type, const GLvoid* pixels);
    GLAPI void APIENTRY impl_glTexImage2D(GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLint border, GLenum format, GLenum type, const GLvoid* pixels);
    GLAPI void APIENTRY impl_glTexParameterf(GLenum target, GLenum pname, GLfloat param);
    GLAPI void APIENTRY impl_glTexParameterfv(GLenum target, GLenum pname, const GLfloat* params);
    GLAPI void APIENTRY impl_glTexParameteri(GLenum target, GLenum pname, GLint param);
    GLAPI void APIENTRY impl_glTexParameteriv(GLenum target, GLenum pname, const GLint* params);
    GLAPI void APIENTRY impl_glTranslated(GLdouble x, GLdouble y, GLdouble z);
    GLAPI void APIENTRY impl_glTranslatef(GLfloat x, GLfloat y, GLfloat z);
    GLAPI void APIENTRY impl_glVertex2d(GLdouble x, GLdouble y);
    GLAPI void APIENTRY impl_glVertex2dv(const GLdouble* v);
    GLAPI void APIENTRY impl_glVertex2f(GLfloat x, GLfloat y);
    GLAPI void APIENTRY impl_glVertex2fv(const GLfloat* v);
    GLAPI void APIENTRY impl_glVertex2i(GLint x, GLint y);
    GLAPI void APIENTRY impl_glVertex2iv(const GLint* v);
    GLAPI void APIENTRY impl_glVertex2s(GLshort x, GLshort y);
    GLAPI void APIENTRY impl_glVertex2sv(const GLshort* v);
    GLAPI void APIENTRY impl_glVertex3d(GLdouble x, GLdouble y, GLdouble z);
    GLAPI void APIENTRY impl_glVertex3dv(const GLdouble* v);
    GLAPI void APIENTRY impl_glVertex3f(GLfloat x, GLfloat y, GLfloat z);
    GLAPI void APIENTRY impl_glVertex3fv(const GLfloat* v);
    GLAPI void APIENTRY impl_glVertex3i(GLint x, GLint y, GLint z);
    GLAPI void APIENTRY impl_glVertex3iv(const GLint* v);
    GLAPI void APIENTRY impl_glVertex3s(GLshort x, GLshort y, GLshort z);
    GLAPI void APIENTRY impl_glVertex3sv(const GLshort* v);
    GLAPI void APIENTRY impl_glVertex4d(GLdouble x, GLdouble y, GLdouble z, GLdouble w);
    GLAPI void APIENTRY impl_glVertex4dv(const GLdouble* v);
    GLAPI void APIENTRY impl_glVertex4f(GLfloat x, GLfloat y, GLfloat z, GLfloat w);
    GLAPI void APIENTRY impl_glVertex4fv(const GLfloat* v);
    GLAPI void APIENTRY impl_glVertex4i(GLint x, GLint y, GLint z, GLint w);
    GLAPI void APIENTRY impl_glVertex4iv(const GLint* v);
    GLAPI void APIENTRY impl_glVertex4s(GLshort x, GLshort y, GLshort z, GLshort w);
    GLAPI void APIENTRY impl_glVertex4sv(const GLshort* v);
    GLAPI void APIENTRY impl_glViewport(GLint x, GLint y, GLsizei width, GLsizei height);
    // -------------------------------------------------------

    // Open GL 1.1
    // -------------------------------------------------------
    GLAPI GLboolean APIENTRY impl_glAreTexturesResident(GLsizei n, const GLuint* textures, GLboolean* residences);
    GLAPI void APIENTRY impl_glArrayElement(GLint i);
    GLAPI void APIENTRY impl_glBindTexture(GLenum target, GLuint texture);
    GLAPI void APIENTRY impl_glColorPointer(GLint size, GLenum type, GLsizei stride, const GLvoid* pointer);
    GLAPI void APIENTRY impl_glCopyTexImage1D(GLenum target, GLint level, GLenum internalFormat, GLint x, GLint y, GLsizei width, GLint border);
    GLAPI void APIENTRY impl_glCopyTexImage2D(GLenum target, GLint level, GLenum internalFormat, GLint x, GLint y, GLsizei width, GLsizei height, GLint border);
    GLAPI void APIENTRY impl_glCopyTexSubImage1D(GLenum target, GLint level, GLint xoffset, GLint x, GLint y, GLsizei width);
    GLAPI void APIENTRY impl_glCopyTexSubImage2D(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint x, GLint y, GLsizei width, GLsizei height);
    GLAPI void APIENTRY impl_glDeleteTextures(GLsizei n, const GLuint* textures);
    GLAPI void APIENTRY impl_glDisableClientState(GLenum cap);
    GLAPI void APIENTRY impl_glDrawArrays(GLenum mode, GLint first, GLsizei count);
    GLAPI void APIENTRY impl_glDrawElements(GLenum mode, GLsizei count, GLenum type, const GLvoid* indices);
    GLAPI void APIENTRY impl_glEdgeFlagPointer(GLsizei stride, const GLvoid* pointer);
    GLAPI void APIENTRY impl_glEnableClientState(GLenum cap);
    GLAPI void APIENTRY impl_glGenTextures(GLsizei n, GLuint* textures);
    GLAPI void APIENTRY impl_glGetPointerv(GLenum pname, GLvoid** params);
    GLAPI GLboolean APIENTRY impl_glIsTexture(GLuint texture);
    GLAPI void APIENTRY impl_glIndexPointer(GLenum type, GLsizei stride, const GLvoid* pointer);
    GLAPI void APIENTRY impl_glIndexub(GLubyte c);
    GLAPI void APIENTRY impl_glIndexubv(const GLubyte* c);
    GLAPI void APIENTRY impl_glInterleavedArrays(GLenum format, GLsizei stride, const GLvoid* pointer);
    GLAPI void APIENTRY impl_glNormalPointer(GLenum type, GLsizei stride, const GLvoid* pointer);
    GLAPI void APIENTRY impl_glPolygonOffset(GLfloat factor, GLfloat units);
    GLAPI void APIENTRY impl_glPopClientAttrib(void);
    GLAPI void APIENTRY impl_glPrioritizeTextures(GLsizei n, const GLuint* textures, const GLclampf* priorities);
    GLAPI void APIENTRY impl_glPushClientAttrib(GLbitfield mask);
    GLAPI void APIENTRY impl_glTexCoordPointer(GLint size, GLenum type, GLsizei stride, const GLvoid* pointer);
    GLAPI void APIENTRY impl_glTexSubImage1D(GLenum target, GLint level, GLint xoffset, GLsizei width, GLenum format, GLenum type, const GLvoid* pixels);
    GLAPI void APIENTRY impl_glTexSubImage2D(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLenum type, const GLvoid* pixels);
    GLAPI void APIENTRY impl_glVertexPointer(GLint size, GLenum type, GLsizei stride, const GLvoid* pointer);
    // -------------------------------------------------------

    // Open GL 1.2
    // -------------------------------------------------------
    GLAPI void APIENTRY impl_glDrawRangeElements(GLenum mode, GLuint start, GLuint end, GLsizei count, GLenum type, const GLvoid* indices);
    GLAPI void APIENTRY impl_glTexImage3D(GLenum target, GLint level, GLint internalFormat, GLsizei width, GLsizei height, GLsizei depth, GLint border, GLenum format, GLenum type, const GLvoid* data);
    GLAPI void APIENTRY impl_glTexSubImage3D(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLenum type, const GLvoid* pixels);
    GLAPI void APIENTRY impl_glCopyTexSubImage3D(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLint x, GLint y, GLsizei width, GLsizei height);
    // -------------------------------------------------------

    // Open GL 1.3
    // -------------------------------------------------------
    GLAPI void APIENTRY impl_glActiveTexture(GLenum texture);
    GLAPI void APIENTRY impl_glSampleCoverage(GLfloat value, GLboolean invert);
    GLAPI void APIENTRY impl_glCompressedTexImage3D(GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth, GLint border, GLsizei imageSize, const GLvoid* data);
    GLAPI void APIENTRY impl_glCompressedTexImage2D(GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLint border, GLsizei imageSize, const GLvoid* data);
    GLAPI void APIENTRY impl_glCompressedTexImage1D(GLenum target, GLint level, GLenum internalformat, GLsizei width, GLint border, GLsizei imageSize, const GLvoid* data);
    GLAPI void APIENTRY impl_glCompressedTexSubImage3D(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLsizei imageSize, const GLvoid* data);
    GLAPI void APIENTRY impl_glCompressedTexSubImage2D(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLsizei imageSize, const GLvoid* data);
    GLAPI void APIENTRY impl_glCompressedTexSubImage1D(GLenum target, GLint level, GLint xoffset, GLsizei width, GLenum format, GLsizei imageSize, const GLvoid* data);
    GLAPI void APIENTRY impl_glGetCompressedTexImage(GLenum target, GLint level, GLvoid* pixels);
    GLAPI void APIENTRY impl_glClientActiveTexture(GLenum texture);
    GLAPI void APIENTRY impl_glMultiTexCoord1d(GLenum target, GLdouble s);
    GLAPI void APIENTRY impl_glMultiTexCoord1dv(GLenum target, const GLdouble* v);
    GLAPI void APIENTRY impl_glMultiTexCoord1f(GLenum target, GLfloat s);
    GLAPI void APIENTRY impl_glMultiTexCoord1fv(GLenum target, const GLfloat* v);
    GLAPI void APIENTRY impl_glMultiTexCoord1i(GLenum target, GLint s);
    GLAPI void APIENTRY impl_glMultiTexCoord1iv(GLenum target, const GLint* v);
    GLAPI void APIENTRY impl_glMultiTexCoord1s(GLenum target, GLshort s);
    GLAPI void APIENTRY impl_glMultiTexCoord1sv(GLenum target, const GLshort* v);
    GLAPI void APIENTRY impl_glMultiTexCoord2d(GLenum target, GLdouble s, GLdouble t);
    GLAPI void APIENTRY impl_glMultiTexCoord2dv(GLenum target, const GLdouble* v);
    GLAPI void APIENTRY impl_glMultiTexCoord2f(GLenum target, GLfloat s, GLfloat t);
    GLAPI void APIENTRY impl_glMultiTexCoord2fv(GLenum target, const GLfloat* v);
    GLAPI void APIENTRY impl_glMultiTexCoord2i(GLenum target, GLint s, GLint t);
    GLAPI void APIENTRY impl_glMultiTexCoord2iv(GLenum target, const GLint* v);
    GLAPI void APIENTRY impl_glMultiTexCoord2s(GLenum target, GLshort s, GLshort t);
    GLAPI void APIENTRY impl_glMultiTexCoord2sv(GLenum target, const GLshort* v);
    GLAPI void APIENTRY impl_glMultiTexCoord3d(GLenum target, GLdouble s, GLdouble t, GLdouble r);
    GLAPI void APIENTRY impl_glMultiTexCoord3dv(GLenum target, const GLdouble* v);
    GLAPI void APIENTRY impl_glMultiTexCoord3f(GLenum target, GLfloat s, GLfloat t, GLfloat r);
    GLAPI void APIENTRY impl_glMultiTexCoord3fv(GLenum target, const GLfloat* v);
    GLAPI void APIENTRY impl_glMultiTexCoord3i(GLenum target, GLint s, GLint t, GLint r);
    GLAPI void APIENTRY impl_glMultiTexCoord3iv(GLenum target, const GLint* v);
    GLAPI void APIENTRY impl_glMultiTexCoord3s(GLenum target, GLshort s, GLshort t, GLshort r);
    GLAPI void APIENTRY impl_glMultiTexCoord3sv(GLenum target, const GLshort* v);
    GLAPI void APIENTRY impl_glMultiTexCoord4d(GLenum target, GLdouble s, GLdouble t, GLdouble r, GLdouble q);
    GLAPI void APIENTRY impl_glMultiTexCoord4dv(GLenum target, const GLdouble* v);
    GLAPI void APIENTRY impl_glMultiTexCoord4f(GLenum target, GLfloat s, GLfloat t, GLfloat r, GLfloat q);
    GLAPI void APIENTRY impl_glMultiTexCoord4fv(GLenum target, const GLfloat* v);
    GLAPI void APIENTRY impl_glMultiTexCoord4i(GLenum target, GLint s, GLint t, GLint r, GLint q);
    GLAPI void APIENTRY impl_glMultiTexCoord4iv(GLenum target, const GLint* v);
    GLAPI void APIENTRY impl_glMultiTexCoord4s(GLenum target, GLshort s, GLshort t, GLshort r, GLshort q);
    GLAPI void APIENTRY impl_glMultiTexCoord4sv(GLenum target, const GLshort* v);
    GLAPI void APIENTRY impl_glLoadTransposeMatrixf(const GLfloat* m);
    GLAPI void APIENTRY impl_glLoadTransposeMatrixd(const GLdouble* m);
    GLAPI void APIENTRY impl_glMultTransposeMatrixf(const GLfloat* m);
    GLAPI void APIENTRY impl_glMultTransposeMatrixd(const GLdouble* m);
    // -------------------------------------------------------

    // Some extensions
    // -------------------------------------------------------
    GLAPI void APIENTRY impl_glLockArrays(GLint first, GLsizei count);
    GLAPI void APIENTRY impl_glUnlockArrays();
    GLAPI void APIENTRY impl_glActiveStencilFaceEXT(GLenum face);
    GLAPI void APIENTRY impl_glBlendEquation(GLenum mode);
    GLAPI void APIENTRY impl_glBlendFuncSeparate(GLenum sfactorRGB, GLenum dfactorRGB, GLenum sfactorAlpha, GLenum dfactorAlpha);
    // -------------------------------------------------------

    // OpenGL ES 1.1 extensions
    // -------------------------------------------------------
    GLAPI void APIENTRY impl_glPointParameterf(GLenum pname, GLfloat param);
    GLAPI void APIENTRY impl_glPointParameterfv(GLenum pname, const GLfloat* params);
    // -------------------------------------------------------
#ifdef __cplusplus
}
#endif

#endif /* GL_IMPL_H */
