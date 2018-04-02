getconfig();
function getconfig() {
    var data = {
        "requestUrl": "get_systemconfigure_cfg",
        "method": "get"
    }
    postAjax(data,configstatus);
}
function configstatus(res) {
    if(res.data.configured==0){
        location.href='./firstSettings.html?t='+Math.random();
    }
}
$('.logout').click(function () {
    window.localStorage.removeItem('qtec_router_token');
    location.href='./login.html?redirect='+location.pathname+'?t='+Math.random()+location.hash;
})
// 在接收到数据后做统一处理
function GetQueryString(name)
{
    var reg = new RegExp("(^|&)"+ name +"=([^&]*)(&|$)");
    var r = location.search.substr(1).match(reg);
    if(r!=null){
        return  unescape(r[2]);
    }
}
$.ajaxSetup({
    beforeSend:function(xhr) {
        var token=window.localStorage.getItem('qtec_router_token');
        if(token){
            xhr.setRequestHeader('token_id',token) ;
        }
    },
});
function fadeIn(e) {
    e.removeClass('hide');
    e.fadeIn();
}
function fadeOut(e) {
    e.addClass('hide');
    e.fadeOut();
}
function slideDown(e) {
    e.slideDown("normal");
}
function slideUp(e) {
    e.slideUp("normal");
}
function removeShadow(m) {
    $(m).fadeIn();
    $('.popup-shadow').removeClass('hide');
}
function addShadow(m) {
    $(m).fadeOut();
    $('.popup-shadow').addClass('hide');
}
function addHide(m) {
    $(m).hide();
    $('.popup-shadow').addClass('hide');
}
$('.close-icon').click(function () {
    $(this).parent().parent().fadeOut();
    $('.popup-shadow').addClass('hide');
})

function disLoading() {
    $('.popup-shadow').addClass('hide');
    $('.result-content').hide();
    $('.result-icon').addClass('hide');
    $('.result-icon').removeClass('loading-img');
}
function resultShowIn(icon) {
    $('.result-icon').addClass(icon);
    $('.result-icon').removeClass('hide');
    $('.popup-shadow').removeClass('hide');
    $('.result-content').fadeIn();
}
function resultTipshow(m,n) {
    var icon;
    if(n==-1){
        $('.result-tip').html('正在保存，请稍候...');
    }else {
        $('.result-tip').html(n);
    }
    if(m==-1){
        icon='loading-img';
        resultShowIn(icon)
    }else {
        $('.result-icon').removeClass('loading-img');
        if(m==0){
            icon='failed-icon';
        }
        if(m==1){
            icon='success-icon';
        }
        resultShowIn(icon)
        setTimeout(function () {
            resultTiphide(icon)
        },2000)
    }
}

function resultTiphide(icon) {
    $('.popup-shadow').addClass('hide');
    $('.result-content').fadeOut();
    $('.result-tip').html();
    if(icon){
        setTimeout(function () {
            $(icon).addClass('hide')
            $('.result-icon').removeClass(icon);
        },500)
    }
}


function getHtml(m) {
    $.ajax({
        type: "get",
        url: "/" + m + ".html?t="+Math.random(),
        dataType: "html",
        error: function (error) {
            console.log(error)
            return;
        },
        success: function (res) {
            if(res.length>0){
                $('.frame-list').html('');
                $('.frame-list').append(res);
                // location.hash='#'+m;
            }else {

            }
        }
    });
}