
        $Id$


        SMS Center Project, Root directory

        docs - документация
        src - исходные тексты
        bin - различные бинарные файлы (архив, релизы, ... )


        для компиляции, тестирования и инсталяции, нужно определить три переменных среды
        SMSC_BUILDDIR   - каталог куда происходит компиляция
        SMSC_SRCDIR     - каталог где находятся сырки
        SMSC_INSTALLDIR - куда будет производится инсталяция системы

        для выплнения update,build_update,commit нужно определить
        CVSROOT=:pserver:<user>@src.novosoft.ru:/cvsroot/main
        и хотя бы один раз выполнить cvs login
