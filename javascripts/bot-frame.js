/*
	loadLayout module for GitHub Page :-)

	BOT Man, 2017 (MIT License)
*/

function FileLoader() { }

FileLoader.prototype._get = function (url, callback) {
    var xhr = new XMLHttpRequest();
    xhr.open('GET', url, true);
    xhr.onreadystatechange = function () {
        if (xhr.readyState != 4) return;
        if (xhr.status == 200) callback(xhr.responseText);
        else callback(null);
    };
    xhr.send();
};

FileLoader.prototype.loading = function (filename) {
    return 'Loading <em>' + filename + '</em> 😇';
};

FileLoader.prototype.failed = function (filename) {
    if (document && document.title) document.title = 'Loading Failed';
    return '<p>Loading <a href="' + filename + '"><em>' +
        filename + '</em></a> <strong>Failed</strong> 😅</p>' +
        '<p>👉👉👉 <a href="javascript:void(0)" onclick="location.reload();">' +
        'Try again</a> 👈👈👈</p>';
};

FileLoader.prototype.done = function (filename) {
    return '';
};

FileLoader.prototype.loadFile = function (filename, promptState, done) {
    var that = this;
    if (promptState) promptState(this.loading(filename));

    this._get(encodeURI(filename), function (text) {
        if (text == null) {
            if (promptState) promptState(that.failed(filename));
            return;
        }

        if (promptState) promptState(that.done(filename));
        if (done) done(text);
    });
};

function ConfigLoader() { }

ConfigLoader.prototype.nav = function (config) {
    var navHtml = '<h2>' + '~/ ';
    for (var page in config.pages) {
        navHtml += '<a href="' + config.pages[page] + '">' + page + '</a>' + ' | ';
    }
    navHtml = navHtml.substr(0, navHtml.length - 3) + '</h2>';
    return navHtml;
};

ConfigLoader.prototype.contact = function (config) {
    var contact = config.contact;
    var contactHtml = '<h1>' + contact.name + '</h1>';
    for (var entry in contact) {
        if (entry == 'name') continue;
        contactHtml += '<p><a href="' + contact[entry].link +
            '"><img src="' + contact[entry].img +
            '" /> - ' + contact[entry].text +
            '</a></p>';
    }
    return contactHtml;
};

ConfigLoader.prototype.footer = function (config) {
    var footerHtml = '';
    for (var entry in config.footer) {
        footerHtml += '<p>' + config.footer[entry] + '</p>';
    }
    return footerHtml;
};

function loadLayout(filename, callback) {
    var configLoader = new ConfigLoader();
    function loadSec(config, sec) {
        document.getElementsByClassName(sec + 'Sec')[0]
            .innerHTML = configLoader[sec](config);
    }

    var mdSec = document.getElementsByClassName('markdown-body')[0];
    function promptState(text) {
        mdSec.innerHTML = text;
    }

    (new FileLoader()).loadFile('/static/config.json', promptState, function (config) {
        try {
            config = JSON.parse(config);

            loadSec(config, 'nav');
            loadSec(config, 'contact');
            loadSec(config, 'footer');

            (new FileLoader()).loadFile(filename, promptState, function (text) {
                if (callback) callback(text, function (title) {
                    document.title = title + ' | ' + config.name;
                });
            });

        } catch (e) {
            promptState('Bad JSON format in <em>' + filename + '</em> 😑');
            document.title = 'Loading Failed';
        }
    });
}

if (typeof module !== 'undefined' && typeof exports === 'object') {
    module.exports = loadLayout;
}
