var gulp = require('gulp'),
    usemin = require('gulp-usemin'),
    connect = require('gulp-connect'),
    watch = require('gulp-watch'),
    minifyJs = require('gulp-uglify'),
    concat = require('gulp-concat'),
    rename = require('gulp-rename'),
    minifyHTML = require('gulp-minify-html'),
    nodemon = require('gulp-nodemon'),
    cssmin = require('gulp-cssmin'),
    minifyCss = require('gulp-minify-css'),
    jshint = require('gulp-jshint');
    localtunnel = require('localtunnel'),
    psi = require('psi'),
    ngrok = require('ngrok'),
    portLocal = 80,
    browserSync = require('browser-sync').create();

// var tunnel = localtunnel(portLocal,{subdomain:"awges"}, function(err, tunnel) {
//     if (err) {
//       console.log("\n\rThere is some error in LOCALTUNNEL ME!");
//     }
//     else{
//       console.log("[LOCALTUNNEL] Localtunnel is running on port:"+portLocal+" at:"+tunnel.url);
//     }
//     tunnel.url;
// });

var paths = {
    scripts: 'src/js/**/*.*',
    styles: 'src/css/**/*.*',
    images: 'src/img/**/*.*',
    fonts: 'src/fonts/**/*.*',
    video: 'src/video/**/*.*',
    pages: 'src/html/**/*.ejs',
    index: 'src/index.ejs',
    bower_fonts: 'src/components/**/*.{ttf,woff,eof,svg,woff2,eot}',
    semantic_fonts: 'src/components/semantic/dist/themes/basic/assets/fonts/*.{ttf,woff,eof,svg,woff2,eot}'
};
gulp.task('copy-video', function() {
    return gulp.src(paths.video)
      .pipe(gulp.dest('dist/video'));
});

gulp.task('copy-fonts', function() {
    return gulp.src(paths.semantic_fonts)
      .pipe(gulp.dest('dist/lib/css/themes/default/assets/fonts'));
});

gulp.task('copy-images', function() {
    return gulp.src(paths.images)
      .pipe(gulp.dest('dist/img'));
});

gulp.task('copy-bower_fonts', function() {
    return gulp.src(paths.bower_fonts)
      .pipe(rename({
          dirname: '/fonts'
      }))
      .pipe(gulp.dest('dist/lib'));
});

gulp.task('browser-sync', function() {
    browserSync.init({
        server: {
            baseDir: "./"
        }
    });
});

gulp.task('min_css', function () {
    gulp.src(paths.styles)
      .pipe(cssmin())
      .pipe(rename({suffix: '.min'}))
      .pipe(gulp.dest('dist/css'));
});

gulp.task('min_html', function () {
    return gulp.src(paths.pages)
      .pipe(minifyHTML())
      .pipe(gulp.dest('dist/pages'));
});

gulp.task('lint', function() {
    gulp.src(paths.scripts)
      .pipe(jshint())
      .pipe(jshint.reporter('default'))
      .pipe(jshint.reporter('fail'));
});

gulp.task('min_js', function() {
    return gulp.src(paths.scripts)
        .pipe(minifyJs())
        .pipe(concat('homestark.min.js'))
        .pipe(gulp.dest('dist/js'));
});

gulp.task('usemin', function() {
    return gulp.src(paths.index)
        .pipe(usemin({
            js: [minifyJs(), 'concat'],
            css: [minifyCss({keepSpecialComments: 0}), 'concat'],
            html: [minifyHTML(),'concat']
            }))
        .pipe(gulp.dest('dist/'));
});

gulp.task('copy-data', ['copy-bower_fonts','copy-images']);
gulp.task('build', ['copy-data','min_css','min_html','lint','min_js','usemin','copy-fonts','copy-video']);

gulp.task('watch', function() {
    gulp.watch([paths.images], ['copy-images']);
    gulp.watch([paths.styles], ['min_css']);
    gulp.watch([paths.scripts], ['lint','min_js']);
    gulp.watch([paths.pages], ['min_html']);
    gulp.watch([paths.index], ['usemin']);
});

gulp.task('nodemon', function() {
    // gulp.task('default');
    nodemon({
      script: 'server.js',
      env: {
        'NODE_ENV': 'development'
      }
    })
      .on('restart');
});


//
// tunnel.on('close', function() {
//     // tunnels are closed
// });

gulp.task('default', ['build','watch','nodemon']);
//gulp.task('default', ['build']);
