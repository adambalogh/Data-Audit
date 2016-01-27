var upload = require('bindings')('upload');

window.ready = false;

window.onload = function() {

  var closeButton = document.getElementById("close");
  closeButton.onclick = function() {
    window.close();
  }

  require('electron').ipcRenderer.on('fileName', function(event, fileName) {
    var title = document.getElementById("title");
    title.appendChild(document.createTextNode("Uploading \"" + fileName + "\""));

    upload.upload(fileName, function(a) {
      window.ready = true;
      var closeButton = document.getElementById("close").style.display = "inline-block";
    });
  });

}

// Only close when file has been uploaded
window.onbeforeunload = function(e) {
  e.returnValue = window.ready;
};