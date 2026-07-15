var Clay = require('@rebble/clay');
var clayConfig = require('./config');
var clay = new Clay(clayConfig);

/*function customClay(minified) {
  var clay = this;
  
  clay.on(clay.EVENTS.AFTER_BUILD, function() {
    // 送信前に16進数文字列を数値形式に変換して渡すためのフック
  });
}*/
