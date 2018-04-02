function scrolltotop() {
    $('html,body').animate({scrollTop: '0px'}, 800);
}
var countnums=(function(){
    var trim=function(strings){
        return (strings||"").replace(/^(\s|\u00A0)+|(\s|\u00A0)+$/g,"");//+表示匹配一次或多次，|表示或者，\s和\u00A0匹配空白字符，/^以……开头，$以……结尾，/g全局匹配,/i忽略大小写
    }
    return function(_str){
        // _str=trim(_str);   //去除字符串的左右两边空格
        var strlength=_str.length;
        if(!strlength){   //如果字符串长度为零，返回零
            return 0;
        }
        var chinese=_str.match(/[\u4e00-\u9fa5]/g); //匹配中文，match返回包含中文的数组
        if(!chinese){
            return strlength;
        }else{
            return strlength+(chinese.length)*2; //计算字符个数
        }
    }
})();
var spacecheck=function (m) {
    if((/\s/g).test(m)){
        return -1;
    }
}
function loading(m) {
    // $(e).attr('disabled','disabled');
    // $(e).addClass('disabled');
    // $(e).find('.disabled').removeClass('hide')
    if(m==0){
        $('.loading-tip').html('正在刷新，请稍候...')
    }
    $('.loading').fadeIn();
}
function disLoading(m) {
    // $(e).removeAttr('disabled');
    // $(e).removeClass('disabled');
    // $(e).find('.disabled').addClass('hide')
    $('.loading').fadeOut();
    if(m==0){
        $('.loading-tip').html('正在保存，请稍候...')
    }
}
function errorShow(m,n) {
    $('.error-info').html(n)
    $('.error-info').addClass('error-color');
    var i=($(m).position().top).toFixed(0);
    var j=($(m).position().left+$(m).outerWidth()+6).toFixed(0);
    var k=($(m).parent().height()).toFixed(0);
    $('.error-tip').attr('style','visibility: visible;top:'+i+'px;left:'+j+'px;height:'+k+'px;line-height:'+k+'px;')
    window.setTimeout(function () {
        $('.error-info').removeClass('error-color');
        window.setTimeout(function () {
            $('.error-info').addClass('error-color');
        },300)
    },250)
}
$('input').on('input',function () {
    if($('.error-info').html().length>0){
        errorhide();
    }
})
$('input[type=checkbox] ').click(function () {
    if($(this).attr('checked')){
        $(this).removeAttr('checked');
    }else {
        $(this).attr('checked',true);
    }
})
function errorhide() {
    $('.error-info').html('')
    $('.error-tip').removeAttr('style')
}
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
function resultTipshow(m,n) {
    if(m==0){
        var icon='.failed-icon';
    }
    if(m==1){
        var icon='.success-icon';
    }
    $(icon).removeClass('hide');
    $('.result-tip').html(n);
    $('.tipshow').fadeIn();
    setTimeout(function () {
        $('.tipshow').fadeOut();
        $('.result-tip').html();
        setTimeout(function () {
            $(icon).addClass('hide')
        },500)
    },2000)
}

// $('.menu ul li a').click(function () {
//     var i=$(this).parent().index();
//     switch (i){
//         case 0:
//             linkJump('home');
//             break;
//         case 1:
//             linkJump('settings');
//             break;
//         case 2:
//             linkJump('extend');
//             break;
//         default:
//             break;
//     }
// })
// function linkJump(m) {
//     $.ajax({
//         type:'get',
//         url:'http://192.168.1.1/'+m+'.html?t='+Math.random(),
//         dataType:'html',
//         success:function (res) {
//             // location.href='./'+m
//             $(document).replaceAll(res);
//         },
//         error:function () {
//             resultTipshow(0,'网络连接异常，请检查');
//         }
//     })
// }

$.ajaxSetup({
    beforeSend:function(xhr) {
        var token=window.localStorage.getItem('token');
        if(token){
            xhr.setRequestHeader('token_id',token) ;
        }
        // if(!xhr.timeOut){
        //     xhr.timeOut=200000
        // }
    },
});
var data = {
    "requestUrl": "get_systemconfigure_cfg",
    "method": "get"
}
$.ajax({
    type:'POST',
    url:'http://192.168.1.1/cgi-bin/web.cgi/',
    data:JSON.stringify(data),
    dataType:'json',
    success:function (res) {
        if(res.errorcode==0){
            if(res.data.configured==0){
                window.location.href='./loading.html';
            }
        }
        else {
            error.error_msg(res);
        }
    },
    // error:function () {
    //     resultTipshow(0,'已超时请检查网络连接状态');
    // }
})
// 在接收到数据后做统一处理
function GetQueryString(name)
{
    var reg = new RegExp("(^|&)"+ name +"=([^&]*)(&|$)");
    var r = window.location.search.substr(1).match(reg);
    if(r!=null){
        return  unescape(r[2]);
        return null;
    }
}
$( document ).ajaxSuccess(function( event, request, settings ) {
    // console.log(request)
    // console.log(window.location)
    if (request.responseJSON && request.responseJSON.errorcode == '-100') {
        window.localStorage.removeItem('token');
        window.location.href='./login.html?redirect='+window.location.href;
    }
});

$( document ).ajaxError(function( event, request, settings ) {
    // console.log(request);
    if (request.errorcode == '-100') {
        window.localStorage.removeItem('token');
        window.location.href='./login.html?redirect='+window.location.href;
    }
    if ((request.errorcode == 400 || request.status ==0 ) && window.location.hash!='#firmware-update'){
        $('.loading').attr('style','display:none;')
        resultTipshow(0,'网络异常，请检查')
    }
});