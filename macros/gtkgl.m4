AC_DEFUN(OV_PATH_GTKGL,
  [
    AC_REQUIRE([AM_PATH_GTK])
    AC_REQUIRE([OV_PATH_GL])
    
    AC_ARG_WITH(gtkgl-prefix, [  --with-gtkgl-prefix=DIR prefix where GtkGLArea is installed ])
    if test -n "${with_gtkgl_pefix}"; then
      gtkgl__Idir="-I${with_gtkgl_prefix}/include"
      gtkgl__Ldir="-L${with_gtkgl_prefix}/lib"
    fi
    
    GTKGL_CFLAGS=""
    GTKGL_LIBS=""
    
    AC_LANG_SAVE
    AC_LANG_C
    AC_CHECK_LIB(gtkgl, gdk_gl_query,
      [
        ac_save_CPPFLAGS="${CPPFLAGS}"
        CPPFLAGS="${CPPFLAGS} ${GTK_CFLAGS} ${GL_CFLAGS} ${gtkgl__Idir}"
        AC_CHECK_HEADER(gtkgl/gtkglarea.h,
          [
            have_gtkgl=yes
            GTKGL_CFLAGS="${GTK_CFLAGS} ${GL_CFLAGS} ${gtkgl__Idir}"
            GTKGL_LIBS="${gtkgl__Ldir} -lgtkgl ${GTK_LIBS} ${GL_LIBS}"
          ],
          have_gtkgl=no
        )
        CPPFLAGS="${ac_save_CPPFLAGS}"
      ],
      have_gtkgl=no,
      ${gtkgl__Ldir} ${GTK_LIBS} ${GL_LIBS}
    )
    AC_LANG_RESTORE
    
    if test "X${have_gtkgl}" = Xyes; then
      ifelse([$1], , :, [$1])
    else
      ifelse([$2], , :, [$2])
    fi
    
    AC_SUBST(GTKGL_CFLAGS)
    AC_SUBST(GTKGL_LIBS)
  ]
)
