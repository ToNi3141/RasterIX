#include "gl.h"
#include "glu.h"
#include <array>
#include <cstdint>
#include <stdio.h>

// Picture exported from GIMP
#define HEADER_PIXEL(data, pixel)                                           \
    {                                                                       \
        pixel[0] = (((data[0] - 33) << 2) | ((data[1] - 33) >> 4));         \
        pixel[1] = ((((data[1] - 33) & 0xF) << 4) | ((data[2] - 33) >> 2)); \
        pixel[2] = ((((data[2] - 33) & 0x3) << 6) | ((data[3] - 33)));      \
        data += 4;                                                          \
    }

class Minimal
{
    static constexpr bool ENABLE_LIGHT { true };
    static constexpr bool ENABLE_BLACK_WHITE { false };
    static constexpr bool ENABLE_MULTI_TEXTURE { true };
    static constexpr bool ENABLE_FOG { false };
    static constexpr bool ENABLE_CLIPPING_PLANE { false };

    static constexpr std::array<float, 4> CLEAR_COLOR { { 135.0f / 255.0f, 206.0f / 255.0f, 235.0f / 255.0f, 0.0f } };

public:
    void init(const uint32_t resolutionW, const uint32_t resolutionH)
    {
        glGetIntegerv(GL_MAX_TEXTURE_UNITS, &m_tmuCount);

        m_textureId = loadTexture(cubeTexture);
        if (m_tmuCount > 1)
        {
            m_multiTextureId = loadTexture(cubeMultiTexture);
        }

        // Setup viewport, depth range, and projection matrix
        glViewport(0, 0, resolutionW, resolutionH);
        glDepthRange(0.0, 1.0);
        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();
        gluPerspective(30.0, static_cast<float>(resolutionW) / static_cast<float>(resolutionH), 1.0, 111.0);

        // Enable depth test and depth mask
        glEnable(GL_DEPTH_TEST);
        glDepthMask(GL_TRUE);

        // Setup one light which will lighten the cube
        GLfloat light_ambient[] = { 0.5, 0.5, 0.5, 0.0 };
        GLfloat light_diffuse[] = { 1.5, 1.5, 1.5, 1.0 };
        GLfloat light_specular[] = { 1.0, 1.0, 1.0, 1.0 };
        GLfloat light_position[] = { 1.0, 3.0, 6.0, 0.0 };

        glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, 8.0f);
        glLightfv(GL_LIGHT0, GL_AMBIENT, light_ambient);
        glLightfv(GL_LIGHT0, GL_DIFFUSE, light_diffuse);
        glLightfv(GL_LIGHT0, GL_SPECULAR, light_specular);
        glLightfv(GL_LIGHT0, GL_POSITION, light_position);

        if (ENABLE_LIGHT)
        {
            glActiveTexture(GL_TEXTURE0);
            glEnable(GL_LIGHT0);
            glEnable(GL_LIGHTING);

            static constexpr float colors[4] = { 0.0, 0.0, 0.0, 0.0 };
            glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
            glTexEnvfv(GL_TEXTURE_ENV, GL_TEXTURE_ENV_COLOR, colors);
        }
        else
        {
            glActiveTexture(GL_TEXTURE0);
            glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
        }

        if (ENABLE_BLACK_WHITE)
        {
            static constexpr float colors[4] = { .7, .7, .7, 0.0 };
            glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE);
            glTexEnvfv(GL_TEXTURE_ENV, GL_TEXTURE_ENV_COLOR, colors);
            glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE0_RGB, GL_TEXTURE);
            glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE1_RGB, GL_CONSTANT);
            glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_RGB, GL_DOT3_RGB);
            glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND0_RGB, GL_SRC_COLOR);
            glTexEnvi(GL_TEXTURE_ENV, GL_RGB_SCALE, 1);
        }

        if (ENABLE_MULTI_TEXTURE)
        {
            if (m_tmuCount > 1)
            {
                glActiveTexture(GL_TEXTURE1);
                glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE);
                glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE0_RGB, GL_TEXTURE);
                glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE1_RGB, GL_PREVIOUS);
                glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_RGB, GL_MODULATE);
                glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND0_RGB, GL_SRC_COLOR);
                glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND1_RGB, GL_SRC_COLOR);
                glTexEnvi(GL_TEXTURE_ENV, GL_RGB_SCALE, 1);
            }
        }

        if (ENABLE_FOG)
        {
            glFogf(GL_FOG_START, 10.0f);
            glFogf(GL_FOG_END, 15.0f);
            glFogi(GL_FOG_MODE, GL_LINEAR);
            glFogfv(GL_FOG_COLOR, CLEAR_COLOR.data());
            glEnable(GL_FOG);
        }
    }

    void draw()
    {
        // Setup clear color and clear the framebuffer
        glDisable(GL_SCISSOR_TEST);
        glClearColor(CLEAR_COLOR[0], CLEAR_COLOR[1], CLEAR_COLOR[2], CLEAR_COLOR[3]);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Test the scissor
        glEnable(GL_SCISSOR_TEST);
        glScissor(0, 50, 200, 46);
        glClearColor(1.0f, 0.0f, 0.0f, 0.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glScissor(200, 50, 200, 48);
        glClearColor(0.0f, 1.0f, 0.0f, 0.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glScissor(400, 50, 200, 50);
        glClearColor(0.0f, 0.0f, 1.0f, 0.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glDisable(GL_SCISSOR_TEST);
        glScissor(200, 200, 200, 100);

        // Setup the model view matrix
        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();

        // set camera
        gluLookAt(12.0f, -2.0f, 5.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f);

        // Rotate cube
        static float t2 = 0;
        t2 += 1.0f;
        glRotatef(t2, 0.0f, 0.0f, 1.0f);

        // Scale the cube a bit to make it a bit bigger
        glScalef(1.5f, 1.5f, 1.5f);

        if constexpr (ENABLE_CLIPPING_PLANE)
        {
            static constexpr GLdouble clipPlane[] = { 0.0, 1.0, 0.0, 0.0 };
            glClipPlane(GL_CLIP_PLANE0, clipPlane);
            glEnable(GL_CLIP_PLANE0);
        }

        // Tex Coords for texture 0
        glActiveTexture(GL_TEXTURE0);
        glEnable(GL_TEXTURE_2D);
        glBindTexture(GL_TEXTURE_2D, m_textureId);
        glClientActiveTexture(GL_TEXTURE0);
        glEnableClientState(GL_TEXTURE_COORD_ARRAY);
        glTexCoordPointer(2, GL_FLOAT, 0, cubeTexCoords.data());

        if (ENABLE_MULTI_TEXTURE)
        {
            if (m_tmuCount > 1)
            {
                // Tex Coords for texture 1
                glActiveTexture(GL_TEXTURE1);
                glEnable(GL_TEXTURE_2D);
                glBindTexture(GL_TEXTURE_2D, m_multiTextureId);
                glClientActiveTexture(GL_TEXTURE1);
                glEnableClientState(GL_TEXTURE_COORD_ARRAY);
                glTexCoordPointer(2, GL_FLOAT, 0, cubeTexCoords.data());
            }
        }

        // Enable normals
        glEnableClientState(GL_NORMAL_ARRAY);
        glNormalPointer(GL_FLOAT, 0, cubeNormals.data());

        // Enable vertex arrays
        glEnableClientState(GL_VERTEX_ARRAY);
        glVertexPointer(3, GL_FLOAT, 0, cubeVerts.data());

        // Draw the cube
        glDrawElements(GL_TRIANGLES, cubeIndex.size(), GL_UNSIGNED_SHORT, cubeIndex.data());
    }

private:
    const std::array<uint16_t, 36> cubeIndex = { {
        // clang-format off
        0, 1, 2, 0, 2, 3, // Face three
        4, 5, 6, 4, 6, 7, // Face five
        8, 9, 10, 8, 10, 11, // Face two
        12, 13, 14, 12, 14, 15, // Face six
        16, 17, 18, 16, 18, 19, // Face one
        20, 21, 22, 20, 22, 23 // Face four
        // clang-format on
    } };

    const std::array<float, 72> cubeVerts = { {
        // clang-format off
        -1.0f, 1.0f, 1.0f,
        -1.0f,-1.0f, 1.0f,
        1.0f,-1.0f, 1.0f,
        1.0f, 1.0f, 1.0f,

        -1.0f, 1.0f,-1.0f,
        -1.0f,-1.0f,-1.0f,
        -1.0f,-1.0f, 1.0f,
        -1.0f, 1.0f, 1.0f,

        1.0f, 1.0f, 1.0f,
        1.0f,-1.0f, 1.0f,
        1.0f,-1.0f,-1.0f,
        1.0f, 1.0f,-1.0f,

        -1.0f, 1.0f,-1.0f,
        -1.0f, 1.0f, 1.0f,
        1.0f, 1.0f, 1.0f,
        1.0f, 1.0f,-1.0f,

        -1.0f,-1.0f, 1.0f,
        -1.0f,-1.0f,-1.0f,
        1.0f,-1.0f,-1.0f,
        1.0f,-1.0f, 1.0f,

        1.0f, 1.0f,-1.0f,
        1.0f,-1.0f,-1.0f,
        -1.0f,-1.0f,-1.0f,
        -1.0f, 1.0f,-1.0f,
        // clang-format on
    } };

    const std::array<float, 48> cubeTexCoords = { {
        // clang-format off
        0.0f, 1.0f,
        0.0f, 0.0f,
        1.0f, 0.0f,
        1.0f, 1.0f,

        1.0f, 0.0f,
        0.0f, 0.0f,
        0.0f, 1.0f,
        1.0f, 1.0f,

        1.0f, 1.0f,
        0.0f, 1.0f,
        0.0f, 0.0f,
        1.0f, 0.0f,

        0.0f, 0.0f,
        0.0f, 1.0f,
        1.0f, 1.0f,
        1.0f, 0.0f,

        0.0f, 1.0f,
        0.0f, 0.0f,
        1.0f, 0.0f,
        1.0f, 1.0f,

        1.0f, 1.0f,
        1.0f, 0.0f,
        0.0f, 0.0f,
        0.0f, 1.0f,
        // clang-format on
    } };

    const std::array<float, 72> cubeNormals = { {
        // clang-format off
        0.0f,  0.0f,  1.0f,
        0.0f,  0.0f,  1.0f,
        0.0f,  0.0f,  1.0f,
        0.0f,  0.0f,  1.0f,

        -1.0f,  0.0f,  0.0f,
        -1.0f,  0.0f,  0.0f,
        -1.0f,  0.0f,  0.0f,
        -1.0f,  0.0f,  0.0f,

        1.0f,  0.0f,  0.0f,
        1.0f,  0.0f,  0.0f,
        1.0f,  0.0f,  0.0f,
        1.0f,  0.0f,  0.0f,

        0.0f,  1.0f,  0.0f,
        0.0f,  1.0f,  0.0f,
        0.0f,  1.0f,  0.0f,
        0.0f,  1.0f,  0.0f,

        0.0f, -1.0f,  0.0f,
        0.0f, -1.0f,  0.0f,
        0.0f, -1.0f,  0.0f,
        0.0f, -1.0f,  0.0f,

        0.0f,  0.0f, -1.0f,
        0.0f,  0.0f, -1.0f,
        0.0f,  0.0f, -1.0f,
        0.0f,  0.0f, -1.0f,
        // clang-format on
    } };

    const char* cubeTexture = "B)G@G*WTGJ_VE:;MFJSSG:_VD*3JA)C>AY[EE*OR>Y/>BZ/N@YWKEK#^D:K[:H+-"
                              "4V:I05.07G\"O:7NZ-DB),4.$5VBM97:[7W\"W9'6\\7V^[;7W);7W+566S0$^?(S1["
                              ",D\"&.TF/1%*88G\"VE*7JK+X\"F:SP@I79?Y/9AIKBBZ'KE:OUG+3_B:#MDJGX>8_9"
                              "?I'27G\"M35^>2ER;1%:7&\"IK0%&587*W:'F`9'6\\4F*L5&2P566S3U^M5F6U4%ZF"
                              "!!!1%2!C(2QO)3!S+3I_/4J/6&>K>8C,?Y#5>(G.CJ#GB)KAG*[W9WG\">X[9=(;-"
                              "=(;';GZ[9'.S5F6E87\"R!Q98,4\"$?(K0:WG!;7O#7VVW86^Y8F^\\8&VZ;GO*:WG#"
                              "45F;14^.,CI\\)S!R*3)T)3!S+#=Z/DN.5&&D4E^D<8#$>8?-GJSR>(;,@9+9?Y#5"
                              "D)_?B)C5CY[><X+\"9G6W$R)D(C%U=X;*:7>]<7_':':^;7O%;GS&:7;#;GO(9'*\\"
                              "3U253E:545:755V=55V=.T2&(RQN(BMM0$F+.T:)-T*%0$J08W\"U>(7*?HO0C)G<"
                              "I++P;7NX:WFW3ER:0$Z..$:&)3)U4%VB;'F^;GO!7&FO7&BQ7FJS7FJU97&\\6&2O"
                              "96VM1T^/+S=W1DZ.<7F[<7J\\25*6*3)V'RAL,CU`+SA\\15\"38&NNDIW@A(_2FJ7F"
                              "BYG74V&>5F2B1U63/DR,6FBH/4J--D.&9G.X:G>\\6F>M6F>M7FJS8&RU97&Z56&J"
                              "?H?)>H/%;'6Y8VZQ:W6[<'J`>(+)AI#7BI3;@8[42%*9%R1I!!!3(B]T)S1Y9'&T"
                              "B935AY+1D)O:A(_.>(/$8FVN4UZA*C5X7&>J:G6X9&ZT9W&W:7.Z:'*Y:W6\\6F2M"
                              ";GW!<X'';'J`9W6];WW%<H#*<7[+=H/0BYCEG:OUB9?A6VFQ+#J\")3-Y%B1J5&.G"
                              ";GR\\E:#?E)_@C9C9F:3E56\"A7VJM)C%T5V*E9G&T7VFO7&:L6F2K7&:M:7.Z8&JQ"
                              "=XO3:'S$:'O&<(/0<X;4?9#>BYWND*+S@I3E>HS=BY[LD*/PH;3_B)SD>X_7:7R`"
                              "6FFK6&:F66:I?8K-B9;97VRO9'&T-T2',C^$>(7*8&VR5F.H97*W6&6J8&VS6VBN"
                              "9H#+7';!7WG%97_-97S-;8/7?)+G@YGN>8_D=8O?CJ/UKL03FJ_]>Y'=>([8CJ+H"
                              "A9382%687FNP<X##=('&8F^R4E^D)S1Y&\"5J97*W7VRQ7VRQ;'F^6VBM7VRR66:K"
                              ";XO6:(/1;(?7;(?99'W19G[5<HKC=H[G@9GRC*3[BJ#UCJ3X@)?FJL(.GK;_>(_6"
                              "AYC=)#-W3ERB1U::/$J046\"D2%:<.DB.*SE_:7>]8&ZT66>M8&ZT4V&G6VFO6VJN"
                              "?)?E<Y#==I'A=9#B;8?=;X?@=H[G=(SGCJ/_A)GS@ICMAYWQ<8;6A)KFBJ#J@97="
                              "J+G`2URC7&JR-4:-,3^'35ZE1E2<05*96VFQ?(W2:7>_56:K5V6M2URA45^G46\"D"
                              "=I#<:X?2;(;4;XC9;H?;=HOC>8[H;X3>>HWH;H+:?8_D?Y'BBY[KC:'IE*;M?I#7"
                              "/E\"9%RER#1UG!!!8*3F#76^X=87/?8_83%RF6&JQ156?-TF0/DZ8-4>.*SN%(#%V"
                              ">)#9;87.:8#-:8#/:7[0<(3:<8+;8W3-A)3K@I+H?HWA8&^_<H+,87*W0E&5\"!E="
                              "%BAQ0E6@8G/!9GG$AICC=HG4<(+-9WK%;H#+97G!4V6N4&2L9':_:W_'87.\\56>N"
                              "?)#6=(C.<83/;8#-:WS-<8'5<X'89G3+>X?<>878>X;7@H[;>H3+1$^20$N,<X##"
                              "B9ODC*+NEZKX@ICD?(_<8GC$=8C5@);B=(?487?!5VJU7'*\\>X[9C*+LA)?B>HS5"
                              "<('%;8#$<(/';X'(;G[*>8G7@Y#@>H?6?XO8BI;A9&ZW66*H3%240DN'14Z(AY'/"
                              "O,T6?9#>@Y;D@)+C?I/C;X34:X#08'7%56JZ26\"O4F>U97K(@ICDCJ3P=HS89'?\""
                              "5&.E0U667G\"Q:'J[>HO/?Y#4F*?KB9C:@([.8FZK-T%]/D=_0$=\\:G\"A5EJ)AHO#"
                              "IK#WD9[N=X;8CY_S?8[E8W?/>8WE5&G#3V2^9W_67G3)<(;:J<$1?Y;E>(W;?9/?"
                              "=H7%CIW=FJSK@Y74?([-@Y74CJ#?@Y/0#AY:!!!(#1A/#QE-'2-4/$!O5UF(8&:;"
                              "8VRRGZKZAY3D=(/5@)#F:GO49'?2<(7?;H/==8WFA9[RFK$$:'_.>H_=C:/O>I#:"
                              "CY[>AY;6D*+CH++SD*/DCJ'B;8#!.DR+!!11\"QM7!1%,!Q)+$1M1$AE.,SEL45B/"
                              "2%*1:G>]@8_7AI;B?8[<97?(9GO-?9/G>([B9'O,<(?6DJ?U>I#:@I;><H;,<X?+"
                              "G:SLE*/CFZWNDJ3E-DF,'C%T$B5H!!!3<H7(LL4%AY;67FRJ$1U:$AQ8$QQ6$QQ6"
                              "3%:20DV,:WFY?HW1:WS!;X'*>8S9=(K6<8?3=8O7I+L$GK+ZI;G_AYK=?I'2H+7U"
                              "G*OMB)?9E:;JDZ3H'C%U!!!4%RMQ25VCH+3ZG;'WLL4+T^4I.$>+!!!1!!!/\"152"
                              "(\"MB.D=]4V*:35V97G\"OB)O<AIG=:G[$9'B^>X_5L\\@-D:3GBY[?FJSKG['NDJ;B"
                              "A)/7>XS0'\"UQ!AE=<87+N=$7H[H#H[P$I;P'R-\\JG[+_@Y;AO<`8OL\\8;GS$'BMN"
                              "\"!5+&B972UB-<H&Y?(S(DJ3AFJSM=8C+?I'5D*3H>X[1:'J[5FBG4V.@&2EE\"QU:"
                              "3%VB\"!E>4F.JL,,+H+/^BJ#LK\\<3JL40L\\L9E*O\\O-0DLLH8B9[NM<D6O<\\=7F^V"
                              ")3)H\"Q=('\"E>7VZD6VNE/U&.5FBI3F&D05284V:J,T2(1%:71E65+SU[(2]L4V.@"
                              "9'6\\K\\((I;?`;H',G;/_F;#]AJ#NDZS]@)GLD:K]G[D,H[T/>I#DGK0%B)WOH++]"
                              "I;3NGZS?0$V#/4R$8'\"L,D2#.4J.:'F^76^V(3-Z#R!E'RYR,C^\"3%J:H:SKDZ+D"
                              "K+\\'I;@\"HK0!E:CV@9;F@9CIC:;YD*K^@ISPC*;Z>9/ID:K^H[T0F:`#<8?;F*OX"
                              "DJ#>I;+H@8W(1U63-42&0U28C9[E\\@0`S=XII[@#97.],#Z&&B=M<7O!C9;:>XC."
                              "K;\\,J+P)IKD)EJO[?Y;G<XS?=8_C=8_CBJ3Z@9OQG[H-G[D,9X#3<HC<CZ;WFZ[["
                              "C9S>J+;S5F2D!!!3-D>.KL$)RMPIJ;L(H;#`S-PKH:[]:G?$GZOTEZ'HL;K`HJ[W"
                              "DJ3WE*G[EZO`CZ7YA)KO?9;J@9OQAJ#V?ICL?YGMB*'T?I?HE:S[F:[^EJOYL,0."
                              "G:OQ/DV197.[R]PEP]0AEZCWEZCYE*;YBYOOK;\\/J[L*A9+AKKL%P\\X6AH[6E*#K"
                              "EJK`FK$%EJP#BJ+YA9WT@9GPA)[TCZG_<XWA<XS?D*GZ:8#-<(;2D:3ODZ7N<X7."
                              "*SR#-D>.FJKVOL`=?(W>BISOKL$5=(;;FJT!D*#TK;\\-GJ[\\AY7?LK\\'FZ7LG:GR"
                              "F*X'H+8/EZT&BZ/\\C:7^A9_W@9OQBZ7YA)WP7'7&=8S9A)KDI[G`<X;*0%&57F^S"
                              "W.\\US^(LL\\42D:3R?8_@=XS>?)#E?Y/HAYSNGK$!M\\D6M<81EZ7M7VRQUN(DK;K_"
                              "D*4#FK(/DZL(CZ@\"F;,-CZH!?9?M@YWQ9'W.BJ'NJ+\\(E*CN@Y38'\"MK&2EF[/P["
                              "[P$`Q-@BEZKXBI_MD*7W@9CI=HS@@9CI=8K<I;L(K+`*J[X$E*7I/4R,>(;$;'JZ"
                              "F*`.H[L9EJX,DJP(I+X8E[()?ICN@)OMDJSZG[@\"F:WS+T*%\"QM89G6OGJKCWNTI"
                              "D*/G@9?AAIOI@9CE<HG:?9;GB:+S=Y#ABZ+QIKT(J;X%GK'UBYW<25F5%B5?+#IW"
                              "B)S[E*D'B9[ZCZ3\\LLD=MLP=I[T,L,<2C*#H?9#41EF<#1]@9':U_```W>XIO,\\-"
                              "@)3<?93A=X[=@YKIEJW^A9SM>(_@C:3UDJ?UI[L'K\\((E:;JGJ[K)S9P,#QU)C5O"
                              "";

    const char* cubeMultiTexture = "````````````````````````]@(RJ+?D7HMZ9J1R:JET9*!Q4H5H37YF4H5H7I=N"
                                   ";[!W?L&%E[S!]`0T\\_`PL;[N0&!H1'!A4H5H5HMJ5HQJ4H9H47UMOLWZ[/DI]0$Q"
                                   "````````````````````````^`@XR]D(:(N29J1S;K!V;:YW5(AI2WME4(%G7)1M"
                                   ";:YV>KN#NL[P^`@X_@HZV>866G\"/1G-B58EI58EI4(-G37UEBY_\"X>\\>_`P\\`P\\_"
                                   "`````````````````````````0T][_PLS=P(K<?BK,G?L,KEG+G.3'EI48%J7YAO"
                                   ";:UWD+NWY?(A`0T]`0T][/DIK;KH0FED279E2WME1G1B06Q?H['>Z_@H`P\\_````"
                                   "`````````````````````````````0T]_@HZ^P<W]`0T^@8V[OLKRM@&CZ7$88]`"
                                   "D+&_TN(._`P\\`P\\__0DY\\/TMXN\\?N<;U06!K.6!<.6!<.%Y;<X.LW>L:`@X^````"
                                   "`````````````````````````````````P\\_`@X^`0T]`0T]_`P\\]0$QV>85S]T+"
                                   "X.X=]`0T`P\\_`0T]\\O\\O]0$Q]@(RV.45;7VF/61=.F)<.%U;/UYIM,'Q]@(R````"
                                   "````````````````````````````````````````````````````_PL[[OLK[?HJ"
                                   "^@8V`@X^`0T]]`0T[OLK^P<W^`@XT]`06F^/0V]@06M?.V)</&%?97>=OLKZUN,3"
                                   "`````````````````````````````````````````````P\\_^P<W[_PLX^`@X^`@"
                                   "\\O\\O_@HZ]@(R[_PL]`0T`0T]_@HZT]`0772/1G-B0VYA/&-=0V]A4'=T3W!X3FU["
                                   "````````````````````````````````````````_`P\\[OLKS]P,K;KJGZS;J[CG"
                                   "SML*Z_@HZ_@H]@(R_`P\\````_`P\\W^P<@)&X17%A2'9C4()H6(YL7Y1T9)=[99E\\"
                                   "`````````````````````````````````````0T][OLKKKKJ0V)M6'U\\78-`6G]^"
                                   "BIW\"U^04]P0S_`P\\`````````@X^[_PLJ+7C1G)E6(YL:ZIU:ZMU:*AT9*%Q7YAN"
                                   "````````````````````````````````````_@HZV.457'F'>+I`<;1X;*UU:ZQV"
                                   "8I5ZS]T,`P\\_````````````````^P<WQM,#4'AR:JEV;[)W;:YV;:]V:JIT8Y]Q"
                                   "`````````````````````````````````@X^\\O\\OP,W];IJ+?\\6#=+AZ<[AZ;[)W"
                                   "?J.GY?(A`P\\_````````````````^`@XQ]0#7'R$89QP89UP79=N7I=N8)EO7I=N"
                                   "`````````````````````````````P\\_]`0TX.T=ML3R8XF(>;Q`=KQ[;[%W:JAV"
                                   "9HJ2V^@8`P\\_`````````````P\\_\\O\\OJ;;E;J*#8Y]R5(AJ4()G4()G5(=I5XQK"
                                   "````````````````````````_`P\\]P,S[/DIXN\\?P<[]67J\":J=W;*UV<+-W=+EZ"
                                   "<:J\"H[7<_0DY`````````````0T]ZO<GE:3/8)=Q9*%R8IYQ6I%L48-G2WME2WME"
                                   "`````````````````````0T]]@(RZ_@H[OPKZO<GM\\/T5'EZ89IQ9Z9S;[%W=KQ["
                                   ">\\\"!<:*-YO,C`0T]`````````@X^[OLKH['?37UF4H9H5(IJ58IJ5HMJ5HQJ5(AI"
                                   "````````````````````^`@X[OLK[OLKZ/4EQM,#;7RG47YO8)EO:ZMU<+-X=+AZ"
                                   "=[U\\;:I\\Q=,!_PL[````````````^04UR-8%1VQL1G1C2GIE3X%G4H9H5(EI4X=I"
                                   "````````````````_`P\\\\/TM\\/TMZO<GN<7U/UYI6(!Y6Y)M:JIU>+Q^>K]`?,\"\""
                                   "?<*#=+&!N,CR_@HZ````````````_`P\\ZO<GI[7B06EC1'!B27=C27AD27AD2WIE"
                                   "`````````````P\\_^P<WY_0D\\_`PUN,3;(:@<Z^\";K!W<;-Y?,1_A\\^)BM&.C-*0"
                                   "A\\V,=;.#RMH%_PL[`````````````P\\__0DYX>X>@I2[/6-?06M?0VY@1'!A1W1B"
                                   "````````````^04UW.D9ZO<G]@(RS-D)9Y.%?L>!@<J#A,N(B-&*B]2.D]>;CM\"7"
                                   ">+B$L,GF[?HI`@X^`````````````````@X^^P<WV.456G../F9?0FU@17%B1G)B"
                                   "`````0T]]`0TX>X>T-\\,]P,S_@HZXN\\?G+;/@LR$A\\^+F-JAD]B:CM62E]*JM-+B"
                                   "SM\\([?HJ`0T]`````````````````````````@X^]`0TRM@'6'2'0&I?/VE?/F=>"
                                   "_`P\\^@8V[_PLXN\\?X.X=_@HZ`P\\_^@8VV.85M<KLK<7AJ</<F,:[L\\WFV.<4[_TL"
                                   "^@8V_`P\\`````````````````````````````````0T]\\_`PRM<&@96W0&A?/69>"
                                   "]`0T]@(R]`0T]0$QXN\\?_@HZ`````0T]^04U\\/TM[?LJ[/DHY?,B[/DI^@8V`0T]"
                                   "`P\\_````````````````````````````````````````_`P\\]0$QW>H9M,+P<H>G"
                                   "Z_@H\\/TM[OLKW.D9Q-(!W^P<W^P<W^P<[/DI^`@X`0T]`0T]_`P\\`0T]`P\\_````"
                                   "`````````````````````````````````````````````````@X^^`@X[_PLVN<7"
                                   "Q=(\"R=8&L+WM>HFU7W61<(\"J56F+8'>4J+;CY?(B`@X^````````````````````"
                                   "`````````````````````````````````````````````````````````0T]_0DY"
                                   "/UML.51H36UU8H]^6XEV681X899U<*Y\\9IA^LL#M^P<W````````````````````"
                                   "````````````````````````````````````````````````````````````````"
                                   "3X!F5HQJ89MP;*QU8)QP89QP::ET<K9Y<K-\\;9*7XN\\?````````````````````"
                                   "````````````````````````````````````````````````````````````````"
                                   "58EJ6I%L8Z!Q;:]V8IYQ:ZQU<K=X=KU[<+-X7H]VP<[]`P\\_````````````````"
                                   "````````````````````````````````````````````````````````````````"
                                   "5HQJ6(]L7IEO9J1S8IYP9:-R;*UU9J1R799N5X9RNLCU`P\\_````````````````"
                                   "````````````````````````````````````````````````````````````````"
                                   "3GQI37YF5XUK7YEO7IEO6Y-M8IUP7YEO4H1IE*G+V.85````````````````````"
                                   "````````````````````````````````````````````````````````````````"
                                   "0V5K1&]A3X%G69!K7YIO69!L69!L:HV4P<_\\Y?(A]`0T````````````````````"
                                   "````````````````````````````````````````````````````````````````"
                                   "?Y.U@92Y27!L4()G8)IO58IJ4X9HJ+CB\\/TM_`P\\`P\\_````````````````````"
                                   "````````````````````````````````````````````````````````````````"
                                   "D:C%S-D(M\\7S26]L7I9N4(%G4(%HP<[\\_PL[````````````````````````````"
                                   "````````````````````````````````````````````````````````````````"
                                   "C:J]W^P;Z_@HK[WK68UN3GYF6X-`U.$0`0T]````````````````````````````"
                                   "````````````````````````````````````````````````````````````````"
                                   "";

    // returns a valid textureID on success, otherwise 0
    GLuint loadTexture(const char* tex)
    {
        static constexpr uint32_t WIDTH = 32;
        static constexpr uint32_t HEIGHT = 32;
        int level = 0;
        int border = 0;

        // data is aligned in byte order
        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

        // request textureID
        GLuint textureID;
        glGenTextures(1, &textureID);

        // bind texture
        glBindTexture(GL_TEXTURE_2D, textureID);

        // Convert from the GIMP format to RGB888
        char pixelBuffer[WIDTH * HEIGHT * 3];
        char* pixelBufferPtr = pixelBuffer;
        for (uint32_t i = 0; i < WIDTH * HEIGHT; i++)
        {
            HEADER_PIXEL(tex, pixelBufferPtr);
            pixelBufferPtr += 3;
        }

        // specify the 2D texture map
        glTexImage2D(GL_TEXTURE_2D, level, GL_RGB, WIDTH, HEIGHT, border, GL_RGB, GL_UNSIGNED_BYTE, pixelBuffer);

        // define how to filter the texture
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

        // define clamping mode
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

        // return unique texture identifier
        return textureID;
    }

    GLint m_tmuCount = 0;
    GLuint m_textureId = 0;
    GLuint m_multiTextureId = 0;
};
